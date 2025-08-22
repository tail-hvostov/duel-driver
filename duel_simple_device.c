#include "duel_simple_device.h"
#include "ssd1306/ssd1306_driver.h"
#include "duel_ops_access.h"

static int fop_open(struct inode *inode, struct file *filp) {
    struct spi_device* device = ssd1306_get_spi_device();
    unsigned long access = 0;
    int result;
    struct duel_simple_filp_data* filp_data;
    if (!device) {
        return -ENODEV;
    }
    if (filp->f_mode & FMODE_WRITE) {
        access |= DUEL_OP_WRITING;
        access |= DUEL_OP_RAW_WRITING;
    }
    result = duel_request_ops(access);
    if (result) {
        return result;
    }
    filp_data = kmalloc(sizeof(struct duel_simple_filp_data), GFP_KERNEL);
    filp_data->access = access;
    filp_data->device = container_of(inode->i_cdev, struct duel_simple_dev, cdev);
    filp->private_data = filp_data;
    return 0;
}

static int fop_release(struct inode *inode, struct file *filp) {
    struct duel_simple_filp_data* filp_data = filp->private_data;
    duel_restore_ops(filp_data->access);
    kfree(filp_data);
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = fop_open,
	.release = fop_release,
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