#include "duel_procfs.h"

static struct proc_dir_entry* proc_entry = NULL;

int duel_init_procfs() {
    proc_entry = proc_create(DUEL_PROC_NAME, 0, NULL, &pscu_proc_fops);
    return NULL == proc_entry;
}

void duel_exit_procfs() {
    if (proc_entry) {
        remove_proc_entry(DUEL_PROC_NAME, NULL);
    }
}
