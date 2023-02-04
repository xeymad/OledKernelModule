#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/err.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include "driver_ssd1306_basic.h"
#include "driver_ssd1306_advance.h"

// Timer Variable
#define TIMEOUT 1000 // milliseconds, one second

#define I2C_BUS_AVAILABLE (1)         // I2C Bus available in our Raspberry Pi
#define SLAVE_DEVICE_NAME ("ssd1306") // Device and Driver Name
#define SSD1306_SLAVE_ADDR (0x3C)     // SSD1306 OLED Slave Address

static struct i2c_adapter *ssd1306_adapter = NULL; // I2C Adapter Structure
extern struct i2c_client *ssd1306_client;          // I2C Client Structure (In our case it is OLED)

/* Function prototypes */
static int ssd1306_probe(struct i2c_client *client,
                         const struct i2c_device_id *id);
static int ssd1306_remove(struct i2c_client *client);

/* I2C device ID */
static const struct i2c_device_id ssd1306_id[] = {{"ssd1306", 0}, {}};
MODULE_DEVICE_TABLE(i2c, ssd1306_id);

/* Function to remove the SSD1306 OLED display */
static int ssd1306_remove(struct i2c_client *client)
{
    printk(KERN_INFO "Removing SSD1306 OLED display at address 0x%x\n",
           client->addr);

    return 0;
}

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

/********************************************************************************************/
/* Module initialization function */
static int __init ssd1306_module_init(void)
{
    int ret = -1;
    ssd1306_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    if (ssd1306_adapter != NULL)
    {
        ssd1306_client = i2c_new_client_device(ssd1306_adapter, &oled_i2c_board_info);
        if (ssd1306_client != NULL)
        {
            i2c_add_driver(&ssd1306_driver);
            ret = 0;
        }
        i2c_put_adapter(ssd1306_adapter);
    }
    pr_info("Driver for ssd1306 added.\n");
    return ret;
}

/* Module exit function */
static void __exit ssd1306_module_exit(void)
{
    ssd1306_basic_deinit();
    i2c_unregister_device(ssd1306_client);
    i2c_del_driver(&ssd1306_driver);
    pr_info("SSD1306 Removed!!!\n");
}

// Thread
static struct task_struct *thread_st;
/* Function executed by kernel thread */
static int update_display(void *unused)
{
    char str[100];
    struct rtc_time t;
    ssd1306_basic_clear();
    while (!kthread_should_stop())
    {
        /* do your work */
        /* do your timer stuff here */

        t = rtc_ktime_to_tm(ktime_get_real());
        sprintf(str, "%ptRs", &t);

        // sprintf(str, "Conteggio: %d\n", count);
        pr_info("%s", str);
        // ssd1306_basic_clear();
        // pr_info("%d",ssd1306_basic_string(0,15,str,strlen(str)+1,0xFFFF, SSD1306_FONT_12));
        ssd1306_basic_string(0, 0, str, strlen(str), 1, SSD1306_FONT_12);
        mdelay(200);
    }
    return 0;
}

/* Function to initialize the SSD1306 OLED display */
static int ssd1306_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
    int ret;
    printk(KERN_INFO "SSD1306 OLED display found at address 0x%x\n", client->addr);
    if(ssd1306_basic_init(SSD1306_INTERFACE_IIC, SSD1306_ADDR_SA0_0))
        goto init_err;
    
    char welcome[50] = "Benvenuti By WuagliunzEmbedded";
    ssd1306_basic_string(0, 15, welcome, strlen(welcome), 1, SSD1306_FONT_12);

    printk(KERN_INFO "Creating Thread\n");
    /* Create the kernel thread with name 'mythread' */
    thread_st = kthread_run(update_display, NULL, "mythread");
    if (IS_ERR(thread_st))
    {
        goto thread_err;
    }
    return 0;

init_err:
    i2c_unregister_device(ssd1306_client);
    return 1;

thread_err:
    printk(KERN_ERR "Thread creation failed\n");
    ssd1306_basic_deinit();
    i2c_unregister_device(ssd1306_client);
    i2c_del_driver(&ssd1306_driver);
    return PTR_ERR(thread_st); 
}


module_init(ssd1306_module_init);
module_exit(ssd1306_module_exit);

MODULE_AUTHOR("GAGDisplay");
MODULE_DESCRIPTION("Linux I2C module for SSD1306 OLED display");
MODULE_LICENSE("GPL");
