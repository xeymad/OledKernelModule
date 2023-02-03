#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
//#include <linux/init.h>
// #include <linux/of.h>
// #include <linux/slab.h>
#include "driver_ssd1306_basic.h"
#include "driver_ssd1306_advance.h"

#define I2C_BUS_AVAILABLE   (          0 )              // I2C Bus available in our Raspberry Pi
#define SLAVE_DEVICE_NAME   (   "ssd1306")              // Device and Driver Name
#define SSD1306_SLAVE_ADDR  (       0x3C )              // SSD1306 OLED Slave Address

static struct i2c_adapter *ssd1306_adapter     = NULL;  // I2C Adapter Structure
extern struct i2c_client  *ssd1306_client;  // I2C Client Structure (In our case it is OLED)


/* Function prototypes */
static int ssd1306_probe(struct i2c_client *client,
                        const struct i2c_device_id *id);
static int ssd1306_remove(struct i2c_client *client);

/* I2C device ID */
static const struct i2c_device_id ssd1306_id[] = {{ "ssd1306", 0 },{ }};
MODULE_DEVICE_TABLE(i2c, ssd1306_id);


/* Function to initialize the SSD1306 OLED display */
static int ssd1306_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
    printk(KERN_INFO "SSD1306 OLED display found at address 0x%x\n", client->addr);
    //ssd1306_basic_init(I2C_BUS_AVAILABLE, SSD1306_ADDR_SA0_0);
    // ssd1306_advance_init(I2C_BUS_AVAILABLE,SSD1306_ADDR_SA0_0);
    //ssd1306_advance_picture( 0, 0, 127, 63, (uint8_t *)gs_image);
    ssd1306_basic_init(I2C_BUS_AVAILABLE, SSD1306_ADDR_SA0_0);
    ssd1306_basic_string(0,15,"testProvaWagliunzEmbedded",strlen("testProvaWagliunzEmbedded")+1,0xFFFF, SSD1306_FONT_12);
    return 0;
}

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
            .name   = SLAVE_DEVICE_NAME,
            .owner  = THIS_MODULE,
        },
        .probe          = ssd1306_probe,
        .remove         = ssd1306_remove,
        .id_table       = ssd1306_id,
};
 
/* I2C Board Info structure */
static struct i2c_board_info oled_i2c_board_info = {
        I2C_BOARD_INFO(SLAVE_DEVICE_NAME, SSD1306_SLAVE_ADDR)
};



/* Module initialization function */
static int __init ssd1306_module_init(void)
{
    int ret = -1;
    ssd1306_adapter= i2c_get_adapter(I2C_BUS_AVAILABLE);
    if( ssd1306_adapter != NULL )
    {
        ssd1306_client = i2c_new_client_device(ssd1306_adapter, &oled_i2c_board_info);
        if( ssd1306_client != NULL )
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
    i2c_unregister_device(ssd1306_client);
    i2c_del_driver(&ssd1306_driver);
    pr_info("Driver Removed!!!\n");
}

module_init(ssd1306_module_init);
module_exit(ssd1306_module_exit);

MODULE_AUTHOR("Xeymad");
MODULE_DESCRIPTION("Linux I2C module for SSD1306 OLED display");
MODULE_LICENSE("GPL");
