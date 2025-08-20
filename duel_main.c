#include <linux/module.h>

#include "duel_main.h"
#include "duel_debug.h"
//#include "duel_ssd1306.h"

static int duel_major = 0;
static int duel_minor = 0;
static int duel_chrdev_count = 3;

static void duel_exit(void) {
    dev_t devno = MKDEV(duel_major, duel_minor);
    PDEBUG("Duel: killing SPI...\n");
    //duel_ssd1306_exit();
    PDEBUG("Duel: cleaning module memory...\n");
    unregister_chrdev_region(devno, duel_chrdev_count);
    //kfree(devices);
}

static int __init duel_init(void) {
    int result = 0;
    dev_t dev;

    PDEBUG("Duel: initialization started...\n");

    //Динамическая инициализация области для символьных устройств.
    result = alloc_chrdev_region(&dev, duel_minor, duel_chrdev_count, "duel");
    if (result < 0) {
        printk(KERN_WARNING "Duel: couldn't choose chrdev_region.\n");
        return result;
    }
    duel_major = MAJOR(dev);

    /*result = duel_ssd1306_init();
    if (result) {
        goto fault;
    }*/

    //devices = kmalloc(sizeof(struct duel_led_device) * duel_chrdev_count, GFP_KERNEL);
    //if (!devices) {
    //    result = -ENOMEM;
    //    goto fault;
    //}

    return 0;
fault:
    duel_exit();
    return result;
}

module_init(duel_init);
module_exit(duel_exit);

MODULE_LICENSE("Dual BSD/GPL");
