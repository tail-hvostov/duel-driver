#include "duel_str_device.h"

static struct file_operations fops = {
    .owner = THIS_MODULE,
    //.open = pscu_open,
	//.release = pscu_release,
	//.write = pscu_write,
	//.read = pscu_read
};

//Устанавливает NULL в случае неудачи.
int duel_alloc_str_dev(struct duel_str_dev** device, int major, int minor) {
    dev_t devno = MKDEV(major, minor);
    struct duel_str_dev* instance;
    int error;
    instance = kmalloc(sizeof(struct duel_str_dev), GFP_KERNEL);
    if (!instance) {
        printk(KERN_WARNING "Duel: out of memory.\n");
        *device = NULL;
        return -ENOMEM;
    }
    *device = instance;
    cdev_init(&instance->cdev, &fops);
	instance->cdev.owner = THIS_MODULE;
	instance->cdev.ops = &fops;
    error = cdev_add(&instance->cdev, devno, 1);
    if (error) {
		printk(KERN_WARNING "Duel: device #%d allocation failed with the code %d.\n", minor, error);
	}
    return 0;
}

//Может безопасно получать NULL.
void duel_free_str_dev(struct duel_str_dev* device) {
    if (device != NULL) {
        cdev_del(&device->cdev);
        kfree(device);
    }
}