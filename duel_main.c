#include <linux/module.h>

#include "duel_main.h"
#include "duel_debug.h"

#define CHARDEV_COUNT 3
#define DUEL_MODULE_NAME "duel"

static int char_major = 0;
static int char_minor = 0;

static void duel_exit(void) {
    dev_t devno = MKDEV(char_major, char_minor);
    PDEBUG("Duel: killing SPI...\n");
    //duel_ssd1306_exit();
    PDEBUG("Duel: cleaning module memory...\n");
    unregister_chrdev_region(devno, CHARDEV_COUNT);
    //kfree(devices);
}

static int __init duel_init(void) {
    int result = 0;
    dev_t dev;

    PDEBUG("Duel: initialization started...\n");

    //Динамическая инициализация области для символьных устройств.
    result = alloc_chrdev_region(&dev, char_minor, CHARDEV_COUNT, DUEL_MODULE_NAME);
    if (result < 0) {
        printk(KERN_WARNING "Duel: couldn't allocate chrdev_region.\n");
        return result;
    }
    char_major = MAJOR(dev);

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
