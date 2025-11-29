#include "duel_str_device.h"
#include "ssd1306/ssd1306_driver.h"
#include "duel_ops_access.h"
#include "ssd1306/ssd1306_device.h"

#define FAST_SYM_SIZE 5
typedef u8 fast_sym[FAST_SYM_SIZE];
#include "font-generator/encoded.txt"

//Сначала идут латинские символы
//Потом цифры
//Потом псведо-символ
#define ALPHABET_COUNT 26
#define HOR_GAP 1
#define PSEUDO_SYM_INDEX (FAST_SYM_COUNT - 1)

static char* usr_buf = NULL;
static u16 usr_buf_size;
static u16 syms_per_line;
static u16 new_line_jump;

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
                    syms_per_line = config->width / (FAST_SYM_SIZE + HOR_GAP);
                    usr_buf_size = syms_per_line * ssd1306_get_display_pages(config);
                    new_line_jump = config->width % (FAST_SYM_SIZE + HOR_GAP);
                    usr_buf = kmalloc(usr_buf_size, GFP_KERNEL);
                    memset(usr_buf, ' ', usr_buf_size);
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

static ssize_t fop_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    struct spi_device* device = ssd1306_get_spi_device();
    size_t remaining_bytes;
    ssize_t result;
    unsigned int first_page, last_page;
    u16 i;
    char* cur_sym;
    u8* graphics_buf;
    if (!device) {
        return -ENODEV;
    }
    remaining_bytes = usr_buf_size - *f_pos;
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

    graphics_buf = ssd1306_get_graphics_buf(device) + (*f_pos * FAST_SYM_SIZE);
    cur_sym = usr_buf + *f_pos;
    for (i = 0; i < count; i++) {
        if ((*cur_sym >= 'A') && (*cur_sym <= 'Z')) {
            memcpy(graphics_buf, &fast_syms[*cur_sym - 'A'], FAST_SYM_SIZE);
        }
        else if ((*cur_sym >= 'a') && (*cur_sym <= 'z')) {
            memcpy(graphics_buf, &fast_syms[*cur_sym - 'a'], FAST_SYM_SIZE);
        }
        else if ((*cur_sym >= '0') && (*cur_sym <= '9')) {
            memcpy(graphics_buf, &fast_syms[*cur_sym - '0' + ALPHABET_COUNT], FAST_SYM_SIZE);
        }
        else if (*cur_sym == ' ') {
            memset(graphics_buf, 0, FAST_SYM_SIZE);
        }
        else {
            memcpy(graphics_buf, &fast_syms[PSEUDO_SYM_INDEX], FAST_SYM_SIZE);
        }
        cur_sym++;
        if ((*f_pos + i + 1) % syms_per_line) {
            graphics_buf += FAST_SYM_SIZE + HOR_GAP;
        }
        else {
            graphics_buf += new_line_jump;
        }
    }

    first_page = *f_pos / syms_per_line;
    *f_pos += count;

    last_page = *f_pos / syms_per_line;
    if (!(*f_pos % syms_per_line)) {
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

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = fop_open,
	.release = fop_release,
	.write = fop_write,
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
