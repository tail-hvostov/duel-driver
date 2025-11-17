#include "duel_fast_device.h"
#include "ssd1306/ssd1306_driver.h"
#include "ssd1306/ssd1306_device.h"
#include "ssd1306/ssd1306_graph.h"
#include "duel_ops_access.h"
#include "duel_debug.h"

static int fop_open(struct inode *inode, struct file *filp) {
    struct spi_device* device = ssd1306_get_spi_device();
    unsigned long access = 0;
    int result;
    struct duel_fast_filp_data* filp_data;
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
    filp_data = kmalloc(sizeof(struct duel_fast_filp_data), GFP_KERNEL);
    filp_data->access = access;
    filp_data->device = container_of(inode->i_cdev, struct duel_fast_dev, cdev);
    filp->private_data = filp_data;
    return 0;
}

static int fop_release(struct inode *inode, struct file *filp) {
    struct duel_fast_filp_data* filp_data = filp->private_data;
    duel_restore_ops(filp_data->access);
    kfree(filp_data);
    return 0;
}

static ssize_t fop_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    struct spi_device* device = ssd1306_get_spi_device();
    size_t remaining_bytes;
    ssize_t result;
    unsigned int first_page, last_page;
    u8* graphics_buf;
    if (!device) {
        return -ENODEV;
    }
    remaining_bytes = SSD1306_GRAPHICS_BUF_SIZE - *f_pos;
    count = (count > remaining_bytes) ? remaining_bytes : count;
    if (count <= 0) {
        return 0;
    }
    if (ssd1306_device_lock_interruptible(device)) {
        return -ERESTARTSYS;
    }
    graphics_buf = ssd1306_get_graphics_buf(device) + *f_pos;
    if (copy_from_user(graphics_buf, buf, count)) {
        result = -EFAULT;
        goto out;
    }
    first_page = *f_pos / SSD1306_DISPLAY_WIDTH;
    *f_pos += count;

    last_page = *f_pos / SSD1306_DISPLAY_WIDTH;
    if (!(*f_pos % SSD1306_DISPLAY_WIDTH)) {
        last_page -= 1;
    }

    if (ssd1306_redraw_pages(device, first_page, last_page)) {
        result = -EIO;
        *f_pos -= count;
        goto out;
    }
    result = count;
out:
    ssd1306_device_unlock(device);
    return result;
}

static ssize_t fop_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    struct spi_device* device = ssd1306_get_spi_device();
    size_t remaining_bytes;
    ssize_t result;
    u8* graphics_buf;
    if (!device) {
        return -ENODEV;
    }
    remaining_bytes = SSD1306_GRAPHICS_BUF_SIZE - *f_pos;
    count = (count > remaining_bytes) ? remaining_bytes : count;
    if (count <= 0) {
        return 0;
    }
    if (ssd1306_device_lock_interruptible(device)) {
        return -ERESTARTSYS;
    }
    graphics_buf = ssd1306_get_graphics_buf(device) + *f_pos;
    if (copy_to_user(buf, graphics_buf, count)) {
        result = -EFAULT;
        goto out;
    }
    *f_pos += count;
    result = count;
out:
    ssd1306_device_unlock(device);
    return result;
}

static loff_t fop_llseek(struct file *filp, loff_t off, int whence) {
    loff_t newpos;
    switch(whence) {
    case SEEK_SET://0
        newpos = off;
        break;
    case SEEK_CUR://1
        newpos = filp->f_pos + off;
        break;
    case SEEK_END://2
        newpos = SSD1306_GRAPHICS_BUF_SIZE + off;
        break;
    default:
        return -EINVAL;
    }
    if (newpos < 0) {
        return -EINVAL;
    }
    filp->f_pos = newpos;
    return newpos;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = fop_open,
    .release = fop_release,
    .write = fop_write,
    .read = fop_read,
    .llseek = fop_llseek
};

//Устанавливает NULL в случае неудачи.
int duel_alloc_fast_dev(struct duel_fast_dev** device, int major, int minor) {
    dev_t devno = MKDEV(major, minor);
    struct duel_fast_dev* instance;
    int error;
    instance = kmalloc(sizeof(struct duel_fast_dev), GFP_KERNEL);
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
void duel_free_fast_dev(struct duel_fast_dev* device) {
    if (device != NULL) {
        cdev_del(&device->cdev);
        kfree(device);
    }
}
