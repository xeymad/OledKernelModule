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
#include "driver_ssd1306_basic.h"
#include "driver_ssd1306_advance.h"

//Timer Variable
#define TIMEOUT 1000    //milliseconds, one second

#define I2C_BUS_AVAILABLE   (          1 )              // I2C Bus available in our Raspberry Pi
#define SLAVE_DEVICE_NAME   (   "ssd1306")              // Device and Driver Name
#define SSD1306_SLAVE_ADDR  (       0x3C )              // SSD1306 OLED Slave Address

static struct i2c_adapter *ssd1306_adapter     = NULL;  // I2C Adapter Structure
extern struct i2c_client  *ssd1306_client;  // I2C Client Structure (In our case it is OLED)

static struct timer_list etx_timer;
static unsigned int count = 0;
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

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
    //ssd1306_basic_init(SSD1306_INTERFACE_IIC, SSD1306_ADDR_SA0_0);
    // ssd1306_advance_init(SSD1306_INTERFACE_IIC,SSD1306_ADDR_SA0_0);
    //ssd1306_advance_picture( 0, 0, 127, 63, (uint8_t *)gs_image);
    
    pr_info("%d",ssd1306_basic_init(SSD1306_INTERFACE_IIC, SSD1306_ADDR_SA0_0));
    //pr_info("%d",ssd1306_basic_display_on());
    //ssd1306_basic_init(SSD1306_INTERFACE_IIC, SSD1306_ADDR_SA0_0);
    pr_info("%d",ssd1306_basic_string(0,15,"testProvaWagliunzEmbedded",strlen("testProvaWagliunzEmbedded")+1,0xFFFF, SSD1306_FONT_12));
    //ssd1306_basic_string(0,15,"testProvaWagliunzEmbedded",strlen("testProvaWagliunzEmbedded")+1,0xFFFF, SSD1306_FONT_12);
    return 0;
}

/* Function to remove the SSD1306 OLED display */
static int ssd1306_remove(struct i2c_client *client)
{
    printk(KERN_INFO "Removing SSD1306 OLED display at address 0x%x\n",
            client->addr);

    return 0;
}
 
/*************** Driver Timer functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
/******************************************************/

//File operation structure 
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .release        = etx_release,
};


/*
** This function will be called when we open the Device file
*/ 
static int etx_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp, 
                                    char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read Function\n");
    return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, 
                                const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write function\n");
    return len;
}

//Timer Callback function. This will be called when timer expires
void update_display(struct timer_list * data)
{
    /* do your timer stuff here */
    char str[50];
    sprintf(str, "Conteggio: %d\n", count++);
    pr_info("%s",str);
    //pr_info("%d",ssd1306_basic_string(0,15,str,strlen(str)+1,0xFFFF, SSD1306_FONT_12));
    //ssd1306_basic_string(0,15,str,strlen(str),0xFFFF, SSD1306_FONT_12);
    /*
       Re-enable timer. Because this function will be called only first time. 
       If we re-enable this will work like periodic timer. 
    */
    mod_timer(&etx_timer, jiffies + msecs_to_jiffies(TIMEOUT));
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

//Thread
static struct task_struct *thread_st;

/* Function executed by kernel thread */
static int thread_fn(void *unused)
{
    ssd1306_basic_clear();
    while (!kthread_should_stop()) {
        /* do your work */
        /* do your timer stuff here */
        char str[50];
        sprintf(str, "Conteggio: %d\n", count);
        pr_info("%s",str);
        //ssd1306_basic_clear();
        //pr_info("%d",ssd1306_basic_string(0,15,str,strlen(str)+1,0xFFFF, SSD1306_FONT_12));
        ssd1306_basic_string(0,15,str,strlen(str),0xFFFF, SSD1306_FONT_12);
        mdelay(200);
    }
    return 0;
}


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

    //Timer init
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
            pr_err("Cannot allocate major number\n");
            return -1;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
    /*Creating cdev structure*/
    cdev_init(&etx_cdev,&fops);
 
    /*Adding character device to the system*/
    if((cdev_add(&etx_cdev,dev,1)) < 0){
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }

    /*Creating struct class*/
    if(IS_ERR(dev_class = class_create(THIS_MODULE,"etx_class"))){
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }
 
    /*Creating device*/
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device"))){
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }
 
    /* setup your timer to call my_timer_callback */
    timer_setup(&etx_timer, update_display, 0);       //If you face some issues and using older kernel version, then you can try setup_timer API(Change Callback function's argument to unsingned long instead of struct timer_list *.
 
    /* setup timer interval to based on TIMEOUT Macro */
    mod_timer(&etx_timer, jiffies + msecs_to_jiffies(TIMEOUT));
 
    pr_info("Device Driver Insert...Done!!!\n");

    printk(KERN_INFO "Creating Thread\n");
    /* Create the kernel thread with name 'mythread' */
    thread_st = kthread_run(thread_fn, NULL, "mythread");
    if (IS_ERR(thread_st)) {
        printk(KERN_ERR "Thread creation failed\n");
        return PTR_ERR(thread_st);
    }

    return ret;
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}

/* Module exit function */
static void __exit ssd1306_module_exit(void)
{
    i2c_unregister_device(ssd1306_client);
    i2c_del_driver(&ssd1306_driver);
    pr_info("SSD1306 Removed!!!\n");
    del_timer(&etx_timer);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Timer Remove...Done!!!\n");
}

module_init(ssd1306_module_init);
module_exit(ssd1306_module_exit);

MODULE_AUTHOR("Xeymad");
MODULE_DESCRIPTION("Linux I2C module for SSD1306 OLED display");
MODULE_LICENSE("GPL");
