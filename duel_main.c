#include <linux/module.h>

#include "duel_main.h"
#include "duel_debug.h"
#include "duel_fast_device.h"
#include "duel_simple_device.h"
#include "duel_str_device.h"

#define CHARDEV_COUNT 3
#define DUEL_MODULE_NAME "duel"

static int char_major = 0;
static int char_minor = 0;

static struct duel_fast_dev* fast_dev = NULL;
static struct duel_simple_dev* simple_dev = NULL;
static struct duel_str_dev* str_dev = NULL;

static void duel_exit(void) {
    dev_t devno = MKDEV(char_major, char_minor);
    duel_free_fast_dev(fast_dev);
    duel_free_simple_dev(simple_dev);
    duel_free_str_dev(str_dev);
    unregister_chrdev_region(devno, CHARDEV_COUNT);
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

    //Создание устройств.
    result = duel_alloc_fast_dev(&fast_dev);
    if (result) {
        goto fault;
    }
    result = duel_alloc_simple_dev(&simple_dev);
    if (result) {
        goto fault;
    }
    result = duel_alloc_str_dev(&str_dev);
    if (result) {
        goto fault;
    }

    return 0;
fault:
    duel_exit();
    return result;
}

module_init(duel_init);
module_exit(duel_exit);

MODULE_LICENSE("Dual BSD/GPL");
