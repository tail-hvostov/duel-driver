#include "duel_simple_device.h"

static struct file_operations fops = {
    .owner = THIS_MODULE,
    //.open = pscu_open,
	//.release = pscu_release,
	//.write = pscu_write,
	//.read = pscu_read
};

//Устанавливает NULL в случае неудачи.
int duel_alloc_simple_dev(struct duel_simple_dev** device) {
    struct duel_simple_dev* instance;
    instance = kmalloc(sizeof(struct duel_simple_dev), GFP_KERNEL);
    if (!instance) {
        printk(KERN_WARNING "Duel: out of memory.\n");
        *device = NULL;
        return -ENOMEM;
    }
    *device = instance;
    cdev_init(&device->cdev, &fops);
	device->cdev.owner = THIS_MODULE;
	device->cdev.ops = &fops;
    return 0;
}

//Может безопасно получать NULL.
void duel_free_simple_dev(struct duel_simple_dev* device) {
    if (device != NULL) {
        kfree(device);
    }
}