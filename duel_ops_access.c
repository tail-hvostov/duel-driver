#include "duel_ops_access.h"

#include <linux/spinlock.h>

static DEFINE_SPINLOCK(ops_spinlock);
static unsigned long ops_access = 0;

int duel_request_ops(unsigned long ops) {
    int result = 0;
    spin_lock(&ops_spinlock);

    if (ops_access & ops) {
        result = -EPERM;
    }
    else {
        ops_access = ops_access ^ ops;
    }

    spin_unlock(&ops_spinlock);
    return result;
}

void duel_restore_ops(unsigned long ops) {
    spin_lock(&ops_spinlock);
    ops_access = ops_access ^ ops;
    spin_unlock(&ops_spinlock);
}