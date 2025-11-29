#include "duel_str_device.h"
#include "ssd1306/ssd1306_driver.h"
#include "duel_ops_access.h"

#define FAST_SYM_SIZE 5
typedef u8 fast_sym[FAST_SYM_SIZE];
#include "font-generator/encoded.txt"

#define HOR_GAP 1

static u8* usr_buf = NULL;
static u16 usr_buf_size;

static int prepare_usr_buf(void) {
    int result = 0;
    if (NULL == usr_buf) {
        struct spi_device* device = ssd1306_get_spi_device();
        if (device) {
            if (ssd1306_device_lock_interruptible(device)) {
                result = -ERESTARTSYS;
            }
            else {
                if (NULL == usr_buf) {
                    struct ssd1306_config* config = ssd1306_get_config(device);
                    usr_buf_size = (config->width / (FAST_SYM_SIZE + HOR_GAP))
                                        * ssd1306_get_display_pages(config);
                    usr_buf = kmalloc(usr_buf_size, GFP_KERNEL);
                    if (NULL == usr_buf) {
                        result = -ENOMEM;
                    }
                }
                ssd1306_device_unlock(device);
            }
        }
        else {
            result = -ENODEV;
        }
    }
    return result;
}

static int fop_open(struct inode *inode, struct file *filp) {
    struct spi_device* device = ssd1306_get_spi_device();
    unsigned long access = 0;
    int result;
    struct duel_str_filp_data* filp_data;
    if (!device) {
        return -ENODEV;
    }
    result = prepare_usr_buf();
    if (result) {
        printk(KERN_WARNING "Duel: couldn't allocate memory foor the string device.\n");
        return result;
    }
    if (filp->f_mode & FMODE_WRITE) {
        access |= DUEL_OP_WRITING;
    }
    if (filp->f_mode & FMODE_READ) {
        access |= DUEL_OP_STR_READING;
    }
    result = duel_request_ops(access);
    if (result) {
        return result;
    }
    filp_data = kmalloc(sizeof(struct duel_str_filp_data), GFP_KERNEL);
    filp_data->access = access;
    filp_data->device = container_of(inode->i_cdev, struct duel_str_dev, cdev);
    filp->private_data = filp_data;
    return 0;
}

static int fop_release(struct inode *inode, struct file *filp) {
    struct duel_str_filp_data* filp_data = filp->private_data;
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
    if (usr_buf != NULL) {
        kfree(usr_buf);
    }
}
