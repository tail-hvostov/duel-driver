#include "duel_procfs.h"
#include "ssd1306/ssd1306_driver.h"
#include "ssd1306/ssd1306_device.h"

#include <linux/proc_fs.h>

static struct proc_dir_entry* proc_entry = NULL;
static char* text_buf = NULL;
static size_t text_buf_len = -1;
static struct spi_device* prev_spi = NULL;

static ssize_t proc_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    struct spi_device* spi = ssd1306_get_spi_device();
    if (!spi) {
        return -ENODEV;
    }
    int result = 0;
    if (ssd1306_device_lock_interruptible(spi)) {
        return -ERESTARTSYS;
    }
    if (-1 == text_buf_len) {
        sprintf(text_buf, "width:\nheight:\nmemory_mode: page");
        text_buf_len = strlen(text_buf);
    }
    size_t remaining_bytes = text_buf_len - f_pos;
    size_t count = (count > remaining_bytes) ? remaining_bytes : count;
    if (copy_to_user(buf, text_buf + *f_pos, count)) {
        result = -EFAULT;
        goto ending;
    }
    *f_pos += count;
    result = count;
ending:
    ssd1306_device_unlock(spi);
    return result;
}

static struct proc_ops proc_ops = {
    .proc_read = proc_read
};

int duel_init_procfs(void) {
    text_buf = kmalloc(sizeof(char) * DUEL_PROC_BUF_SIZE, GFP_KERNEL);
    if (!text_buf) {
        return -1;
    }
    proc_entry = proc_create(DUEL_PROC_NAME, 0, NULL, &proc_ops);
    return NULL == proc_entry;
}

void duel_exit_procfs(void) {
    if (proc_entry) {
        remove_proc_entry(DUEL_PROC_NAME, NULL);
    }
    if (text_buf) {
        kfree(text_buf);
    }
}