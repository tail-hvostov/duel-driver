#include "duel_simple_device.h"
#include "ssd1306/ssd1306_driver.h"
#include "ssd1306/ssd1306_device.h"
#include "ssd1306/ssd1306_graph.h"
#include "duel_ops_access.h"
#include "duel_debug.h"

#define BYTE_SIZE 8
#define OUTER_MASK 0x80

static u8* usr_buf = NULL;

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
                    usr_buf = kmalloc(ssd1306_get_graphics_buf_size(config), GFP_KERNEL);
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
    struct duel_simple_filp_data* filp_data;
    if (!device) {
        return -ENODEV;
    }
    result = prepare_usr_buf();
    if (result) {
        printk(KERN_WARNING "Duel: couldn't allocate memory foor the simple device.\n");
        return result;
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

static inline void simple_write(const u8* buf, size_t count, const loff_t* f_pos,
                                unsigned int* first_page, unsigned int* last_page,
                                struct spi_device* device) {
    struct ssd1306_config* config = ssd1306_get_config(device);
    unsigned int bit_line = (8 * *f_pos) / config->width;
    //Горизонтальный бит.
    unsigned int cur_bit = (8 * *f_pos) % config->width;
    size_t outer_i;
    u8 outer;
    u8 inner_bit;
    u8 bit_i;
    u8* page_start;
    u8 mask;

    *first_page = bit_line / 8;

    //Вертикальный бит
    inner_bit = bit_line % 8;
    page_start = ssd1306_get_graphics_buf(device) + *first_page * config->width;
    for (outer_i = 0; outer_i < count; outer_i++) {
        outer = buf[outer_i];
        for (bit_i = 0; bit_i < BYTE_SIZE; bit_i++) {
            //Разберём случай для inner_bit = 4.
            //7 - inner_bit = горизонтальному смещению для сдвига старшего
            //бита на нужную позицию. 
            //В mask я получаю величину вида 000y0000.
            //В page_start[cur_bit] мы изначально имеем величину вида xxxAxxxx.
            //~(0x80 >> 3) = особой маске для зануления А.
            //page_start[cur_bit] & !(0x80 >> 3) = xxx0xxxx.
            //page_start[cur_bit] | mask = xxxyxxxx.
            mask = (outer & OUTER_MASK) >> (BYTE_SIZE - inner_bit - 1);
            page_start[cur_bit] &= ~(OUTER_MASK >> (BYTE_SIZE - inner_bit - 1));
            page_start[cur_bit] |= mask;
            outer = outer << 1;
            cur_bit += 1;
            if (cur_bit == config->width) {
                cur_bit = 0;
                inner_bit += 1;
                bit_line += 1;
                if (inner_bit == BYTE_SIZE) {
                    inner_bit = 0;
                    page_start += config->width;
                }
            }
        }
    }

    *last_page = bit_line / 8;
    //Здесь был выход за границу массива.
    if ((!cur_bit) && (!inner_bit)) {
        *last_page -= 1;
    }
}

static ssize_t fop_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    struct spi_device* device = ssd1306_get_spi_device();
    struct ssd1306_config* config = ssd1306_get_config(device);
    size_t remaining_bytes;
    ssize_t result;
    unsigned int first_page, last_page;
    if (!device) {
        return -ENODEV;
    }
    remaining_bytes = ssd1306_get_graphics_buf_size(config) - *f_pos;
    count = (count > remaining_bytes) ? remaining_bytes : count;
    if (count <= 0) {
        return 0;
    }
    if (ssd1306_device_lock_interruptible(device)) {
        return -ERESTARTSYS;
    }
    if (copy_from_user(usr_buf, buf, count)) {
        result = -EFAULT;
        goto out;
    }
    simple_write(usr_buf, count, f_pos, &first_page, &last_page, device);
    if (ssd1306_redraw_pages(device, first_page, last_page)) {
        result = -EIO;
        goto out;
    }
    result = count;
    *f_pos += count;
out:
    ssd1306_device_unlock(device);
    return result;
}

static inline void simple_read(u8* buf, size_t count, const loff_t* f_pos,
                                struct spi_device* device) {
    struct ssd1306_config* config = ssd1306_get_config(device);
    unsigned int bit_line = (8 * *f_pos) / config->width;
    //Горизонтальный бит.
    unsigned int cur_bit = (8 * *f_pos) % config->width;
    size_t outer_i;
    u8 outer;
    u8 inner_bit;
    u8 bit_i;
    u8* page_start;
    u8 mask;
    //Вертикальный бит
    inner_bit = bit_line % 8;
    page_start = ssd1306_get_graphics_buf(device) + (bit_line / 8) * config->width;
    for (outer_i = 0; outer_i < count; outer_i++) {
        outer = 0;
        for (bit_i = 0; bit_i < BYTE_SIZE; bit_i++) {
            //Разберём случай для inner_bit = 4.
            //outer зануляем сразу, чтобы маски адекватно работали.
            //page_start[cur_bit] имеет значение вида xxxyxxxx.
            //0x80 >> (7 - inner_bit) - особая маска для получеения бита y.
            //000y0000 >> inner_bit = 0000000y.
            mask = page_start[cur_bit] & (OUTER_MASK >> (BYTE_SIZE - inner_bit - 1));
            mask = mask >> inner_bit;
            outer = outer << 1;
            outer |= mask;
            cur_bit += 1;
            if (cur_bit == config->width) {
                cur_bit = 0;
                inner_bit += 1;
                bit_line += 1;
                if (inner_bit == BYTE_SIZE) {
                    inner_bit = 0;
                    page_start += config->width;
                }
            }
        }
        buf[outer_i] = outer;
    }
}

static ssize_t fop_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    struct spi_device* device = ssd1306_get_spi_device();
    struct ssd1306_config* config = ssd1306_get_config(device);
    size_t remaining_bytes;
    ssize_t result;
    if (!device) {
        return -ENODEV;
    }
    remaining_bytes = ssd1306_get_graphics_buf_size(config) - *f_pos;
    count = (count > remaining_bytes) ? remaining_bytes : count;
    if (count <= 0) {
        return 0;
    }
    if (ssd1306_device_lock_interruptible(device)) {
        return -ERESTARTSYS;
    }
    simple_read(usr_buf, count, f_pos, device);
    if (copy_to_user(buf, usr_buf, count)) {
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
    struct spi_device* device = ssd1306_get_spi_device();
    struct ssd1306_config* config = ssd1306_get_config(device);
    loff_t newpos;
    switch(whence) {
    case SEEK_SET://0
        newpos = off;
        break;
    case SEEK_CUR://1
        newpos = filp->f_pos + off;
        break;
    case SEEK_END://2
        newpos = ssd1306_get_graphics_buf_size(config) + off;
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
int duel_alloc_simple_dev(struct duel_simple_dev** device, int major, int minor) {
    dev_t devno = MKDEV(major, minor);
    struct duel_simple_dev* instance;
    int error;
    instance = kzalloc(sizeof(struct duel_simple_dev), GFP_KERNEL);
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
    if (usr_buf != NULL) {
        kfree(usr_buf);
    }
}
