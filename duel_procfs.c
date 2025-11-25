#include "duel_procfs.h"

#include <linux/proc_fs.h>

static struct proc_dir_entry* proc_entry = NULL;

static struct proc_ops proc_ops = {
    .proc_read = NULL//proc_read
};

int duel_init_procfs(void) {
    proc_entry = proc_create(DUEL_PROC_NAME, 0, NULL, &proc_ops);
    return NULL == proc_entry;
}

void duel_exit_procfs(void) {
    if (proc_entry) {
        remove_proc_entry(DUEL_PROC_NAME, NULL);
    }
}
