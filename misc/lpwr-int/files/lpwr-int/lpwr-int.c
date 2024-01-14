/*
 * lpwr_int.c
 * 
 * Low power driver for Denso's 5912 RSU
 * 
 * Copyright (C) 2021 Denso
 *
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/delay.h>


#define LPWR_DEBUG 1
#define debug_printk(fmt, ...) \
            do { if (LPWR_DEBUG) pr_info(fmt, ##__VA_ARGS__); } while (0)

//static char *LPWR_NAME = "vbat_fail_n";
static char *LPWR_NAME = "DCIN Fail";
static int gpio_nb;
static int irq;

int lpwr_irq_triggered = 0;     /* crude lock mechanism */
wait_queue_head_t read_queue;


/* interrupt handler 
 * Comment : Triggered when voltage dip below 9.7V
 *              - Save 256 bytes shared memory to lpwr partition (/dev/mtd10)
 *              - notify lwpr daemon for additional shutdown clean up
 * 
 * Note: 
 *      - 10/04/21 - isr has been refactored to signal lwpr daemon where saving of 
 *                   256 bytes shared memory is done in user space
 *                   this resolved contentions with other flash rw services
 *                 - For kernel/user communication:
 *                    - removed netlink sock - high overhead and latency
 *                    - replaced netlink with sysfs mechanism
 */
static irqreturn_t lpwr_isr(int irq, void *dev_id)
{
    int i;
    debug_printk("Low Power Interrupt received!\n");

    /* trigger for pending read */
    lpwr_irq_triggered = 1;     /* release lock */
    wake_up_interruptible(&read_queue);
    debug_printk("lpwr_irq_triggered\n");

    return IRQ_HANDLED;
}

static int lpwr_dev_open(struct inode *inode, struct file *file)
{
    debug_printk("lpwr_dev_open() is called. \n");
    return 0;
}

static int lpwr_dev_close(struct inode *inode, struct file *file)
{
    debug_printk("lpwr_dev_close() is called. \n");
    return 0;
}

/* Read operation for /dev/lpwrint
 * Comment : 
 *      Not your standard read operation. This is used to signal user application
 *      that an interrupt has occurred as well as pass info back.
 * 
 *      Why this hack and not standard kernel/userspace IPC such as netlink, shared memory, or signal?
 *          1. simple to set up and use with minimal overhead
 *          2. Ultra fast!
 * 
 */
static ssize_t lpwr_dev_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    uint8_t *data = "lpwr: interrupt triggered!\n";
    size_t datalen = strlen(data);
    
    lpwr_irq_triggered = 0;
    
    /* Wait here ...
     * until interrupt occurs 
     */
    wait_event_interruptible(read_queue, lpwr_irq_triggered);
    
    /* User space application register for 'signal' or 'callback'
     * by doing a read
     * this is how the isr signal user app 
     * by sending data
     */
    if (count > datalen) {
        count = datalen;
    }

    if (copy_to_user(buf, data, count)) {
        return -EFAULT;
    }

    lpwr_irq_triggered = 0; /* reset lock */
    
    return count;
}

static ssize_t lpwr_dev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    pr_info("lpwr: Device write\n");
    return 0;
}

/*Declare file operations structure*/
static const struct file_operations lpwr_dev_fops = 
{
    .owner      = THIS_MODULE,
    .open       = lpwr_dev_open,
    .release    = lpwr_dev_close,
    .read       = lpwr_dev_read,
    .write      = lpwr_dev_write
};

static struct miscdevice lpwr_miscdevice = 
{
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "lpwrint",
    .fops   = &lpwr_dev_fops,
};

static int __init lpwr_probe(struct platform_device *pdev)
{
    int ret_val;
    struct device *dev = &pdev->dev;

    dev_info(dev, "lpwr_probe enter \n");

    /* Get int number from device tree */
    gpio_nb = of_get_gpio(pdev->dev.of_node, 0);
    if (gpio_nb < 0)
            return gpio_nb;

    dev_info(dev, "GPIO: %d\n", gpio_nb);
    gpio_request(gpio_nb, LPWR_NAME);
    irq = gpio_to_irq(gpio_nb);
    if (irq < 0)
    {
        dev_err(dev, "IRQ is not available\n");
        return -EINVAL;//1;
    }
    dev_info(dev, "IRQ using gpio_to_irq: %d\n", irq);

    /*Register the interrupt handler*/
    ret_val = devm_request_irq(dev, irq, lpwr_isr, IRQF_SHARED | IRQF_TRIGGER_FALLING, LPWR_NAME, pdev->dev.of_node);
    if(ret_val)
    {
        dev_err(dev, "Failed to request interrupt %d, error %d\n",irq, ret_val);
        return ret_val;
    }
    ret_val = misc_register(&lpwr_miscdevice);
    if (ret_val != 0)
    {
        dev_err(dev, "Could not register the lpwr_miscdevice");
        return ret_val;
    }
    
    /* Init wait queue */
    init_waitqueue_head(&read_queue);
    
    return 0;
}

static int __exit lpwr_remove(struct platform_device *pdev)
{
    gpio_free(gpio_nb);
    misc_deregister(&lpwr_miscdevice);
    dev_info(&pdev->dev,"lpwr_remove() function is called. \n");
    return 0;
}

/*Declare list of devices supported by the driver*/
static const struct of_device_id lpwr_of_ids[] = {
    { .compatible = "fsl,intlpwr"},
    {},
};

MODULE_DEVICE_TABLE(of, lpwr_of_ids);

/*Define platform driver structure*/
static struct platform_driver lpwr_platform_driver = 
{
    .probe  = lpwr_probe,
    .remove = lpwr_remove,
    .driver = 
        {
            .name = "lpwr",
            .of_match_table = lpwr_of_ids,
            .owner = THIS_MODULE,
        }
};

module_platform_driver(lpwr_platform_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Low Power interrupt");
