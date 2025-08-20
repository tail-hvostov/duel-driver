#include "duel_simple_device.h"

static int fop_open(struct inode *inode, struct file *filp) {
    return -ENODEV;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = fop_open,
	//.release = pscu_release,
	//.write = pscu_write,
	//.read = pscu_read
};

//Устанавливает NULL в случае неудачи.
int duel_alloc_simple_dev(struct duel_simple_dev** device, int major, int minor) {
    dev_t devno = MKDEV(major, minor);
    struct duel_simple_dev* instance;
    int error;
    instance = kmalloc(sizeof(struct duel_simple_dev), GFP_KERNEL);
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
void duel_free_simple_dev(struct duel_simple_dev* device) {
    if (device != NULL) {
        cdev_del(&device->cdev);
        kfree(device);
    }
}