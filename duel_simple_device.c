#include "duel_simple_device.h"
#include "ssd1306/ssd1306_driver.h"
#include "ssd1306/ssd1306_device.h"
#include "ssd1306/ssd1306_graph.h"
#include "duel_ops_access.h"
#include "duel_debug.h"

#define BYTE_SIZE 8
#define OUTER_MASK 0x80

//Буфер для файловых операций.
//Мб это лучше, чем каждый раз выделять динамически.
static u8 usr_buf[SSD1306_GRAPHICS_BUF_SIZE];

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

static inline void simple_write(const u8* buf, size_t count, const loff_t* f_pos,
                                unsigned int* first_page, unsigned int* last_page,
                                struct spi_device* device) {
    unsigned int bit_line = (8 * *f_pos) / SSD1306_DISPLAY_WIDTH;
    //Горизонтальный бит.
    unsigned int cur_bit = (8 * *f_pos) % SSD1306_DISPLAY_WIDTH;
    size_t outer_i;
    u8 outer;
    u8 inner_bit;
    u8 bit_i;
    u8* page_start;
    u8 mask;

    *first_page = bit_line / 8;

    //Вертикальный бит
    inner_bit = bit_line % 8;
    page_start = ssd1306_get_graphics_buf(device) + *first_page * SSD1306_DISPLAY_WIDTH;
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
            if (cur_bit == SSD1306_DISPLAY_WIDTH) {
                cur_bit = 0;
                inner_bit += 1;
                bit_line += 1;
                if (inner_bit == BYTE_SIZE) {
                    inner_bit = 0;
                    page_start += SSD1306_DISPLAY_WIDTH;
                }
            }
        }
    }

    *last_page = bit_line / 8;
    if (!cur_bit) {
        *last_page -= 1;
    }
}

static ssize_t fop_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    struct spi_device* device = ssd1306_get_spi_device();
    size_t remaining_bytes;
    ssize_t result;
    unsigned int first_page, last_page;

    if (!device) {
        return -ENODEV;
    }
    remaining_bytes = SSD1306_GRAPHICS_BUF_SIZE - *f_pos;
    count = (count > remaining_bytes) ? remaining_bytes : count;
    if (!count) {
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

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = fop_open,
	.release = fop_release,
	.write = fop_write,
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
