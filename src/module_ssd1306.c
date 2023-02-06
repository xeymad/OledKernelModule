#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include "driver_ssd1306_basic.h"

#define I2C_BUS_AVAILABLE (0)           // I2C Bus available in our Raspberry Pi
#define SLAVE_DEVICE_NAME ("ssd1306")   // Device and Driver Name
#define SSD1306_SLAVE_ADDR (0x3C)       // SSD1306 OLED Slave Address
#define WELCOME "RTSA_Project"          // Welcome string

static struct i2c_adapter *ssd1306_adapter = NULL; // I2C Adapter Structure
extern struct i2c_client *ssd1306_client;          // I2C Client Structure (In our case it is OLED)

/* Function prototypes */
static int ssd1306_probe(struct i2c_client *client,
                         const struct i2c_device_id *id);
static int ssd1306_remove(struct i2c_client *client);
static int __init ssd1306_module_init(void);
static void __exit ssd1306_module_exit(void);
static int update_display(void*);

/* I2C device ID */
static const struct i2c_device_id ssd1306_id[] = {{"ssd1306", 0}, {}};
MODULE_DEVICE_TABLE(i2c, ssd1306_id);

/* I2C driver Structure that has to be added to linux */
static struct i2c_driver ssd1306_driver = {
    .driver = {
        .name = SLAVE_DEVICE_NAME,
        .owner = THIS_MODULE,
    },
    .probe = ssd1306_probe,
    .remove = ssd1306_remove,
    .id_table = ssd1306_id,
};

/* I2C Board Info structure */
static struct i2c_board_info oled_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, SSD1306_SLAVE_ADDR)};

// Thread
static bool stopped = true;
static struct task_struct *kthread_update;
/* Function executed by kernel thread */
static int update_display(void *unused)
{
    stopped = false;
    char date_str[20];
    char time_str[20];
    struct rtc_time t;
    ssd1306_basic_clear();
    while (!kthread_should_stop())
    {
        // Get time from rtc and add +1 GMT
        t = rtc_ktime_to_tm(ktime_get_real() + 3600000000000);
        // Format date string.
        snprintf(date_str, 20, "%04d-%02d-%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        // Format time string.
        snprintf(time_str, 20, "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
        // Print strings on oled
        if (ssd1306_basic_string(0, 0, date_str, strlen(date_str), 1, SSD1306_FONT_12) ||
            ssd1306_basic_string(0, 15, time_str, strlen(time_str), 1, SSD1306_FONT_24))
        {
            pr_info("ssd1306: basic string failed.");
            break;
        }
        // Sleep for 200 ms.
        msleep(200);
    }
    stopped = true;
    pr_info("Exiting from kthread");
    return 0;
}

/********************************************************************************************/
/* Module initialization function */
static int __init ssd1306_module_init(void)
{
    int ret = -1;
    // Find the adapter on the selected bus.
    ssd1306_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    if (ssd1306_adapter != NULL)
    {
        pr_info("Adapter obtained.\n");
        // Find the i2c client target from the adapter
        ssd1306_client = i2c_new_client_device(ssd1306_adapter, &oled_i2c_board_info);
        if (ssd1306_client != NULL)
        {
            pr_info("Client obtained.\n");
            // Add driver for the found i2c client.
            i2c_add_driver(&ssd1306_driver);
            ret = 0;
        }
        pr_info("Put Adapter.\n");
        // Update the adapter.
        i2c_put_adapter(ssd1306_adapter);
        pr_info("Driver for ssd1306 added.\n");
    }
    return ret;
}

/* Function to initialize the SSD1306 OLED display */
static int ssd1306_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    pr_info("SSD1306 OLED display found at address 0x%x\n", client->addr);
    if (ssd1306_basic_init(SSD1306_INTERFACE_IIC, SSD1306_ADDR_SA0_0))
        return 1;
    if(ssd1306_basic_rect(0,16,127,16,1))
        return 1;
    if(ssd1306_basic_string(0, 18, WELCOME, strlen(WELCOME), 1, SSD1306_FONT_12))
        return 1;
    if(ssd1306_basic_rect(0,31,127,31,1))
        return 1;
    pr_info("ssd1306: Creating Thread\n");
    kthread_update = kthread_run(update_display, NULL, "thread_ssd1306");
    if (IS_ERR(kthread_update))
    {
        goto thread_err;
    }
    return 0;
thread_err:
    pr_err("ssd1306: Thread creation failed\n");
    ssd1306_basic_deinit();
    return PTR_ERR(kthread_update);
}

static int ssd1306_remove(struct i2c_client *client)
{
    ssd1306_basic_deinit();
    pr_info("Device ssd1306 removed.");
    return 0;
}

/* Module exit function */
static void __exit ssd1306_module_exit(void)
{
    pr_info("ssd1306: Module Exit function\n");
    if (!stopped)
    {
        pr_info("ssd1306: Deleting thread...\n");
        kthread_stop(kthread_update);
    }
    pr_info("ssd1306: unregister device...\n");
    i2c_unregister_device(ssd1306_client);
    pr_info("ssd1306: removing driver...\n");
    i2c_del_driver(&ssd1306_driver);
    pr_info("Removed driver for ssd1306.\n");
}

module_init(ssd1306_module_init);
module_exit(ssd1306_module_exit);

MODULE_AUTHOR("GAGDisplay");
MODULE_DESCRIPTION("Linux I2C module for SSD1306 OLED display");
MODULE_LICENSE("GPL");
