#include "duel_procfs.h"

#include <linux/proc_fs.h>

static struct proc_dir_entry* proc_entry = NULL;
static char* text_buf = NULL;

static struct proc_ops proc_ops = {
    .proc_read = proc_read
};

int duel_init_procfs(void) {
    text_buf = kmalloc(sizeof(char) * DUEL_PROC_BUF_SIZE, GFP_KERNEL);
    if (NULL == text_buf) {
        return -1;
    }
    proc_entry = proc_create(DUEL_PROC_NAME, 0, NULL, &proc_ops);
    return NULL == proc_entry;
}

void duel_exit_procfs(void) {
    if (NULL != proc_entry) {
        remove_proc_entry(DUEL_PROC_NAME, NULL);
    }
    if (NULL != text_buf) {
        kfree(text_buf);
    }
}

static ssize_t pscu_proc_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
	/*static int need_vals = 1;
	ssize_t result;
	if (need_vals) {
		if (copy_to_user(buf, proc_text, 60)) {
			result = -EFAULT;
			goto ending;
		}
		result = 60;
		f_pos += 60;
	}
	else {
		result = 0;
	}
ending:
	need_vals = !need_vals;
	return result;*/
    return 0;
}