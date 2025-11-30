#include "duel_ops_access.h"

#include <linux/spinlock.h>

static DEFINE_SPINLOCK(ops_spinlock);
static unsigned long ops_access = 0;
static unsigned long ops_stats = 0;

int duel_request_ops(unsigned long ops, unsigned long* stats) {
    int result = 0;
    spin_lock(&ops_spinlock);

    if (ops_access & ops) {
        result = -EPERM;
    }
    else {
        ops_access = ops_access ^ ops;
        if (NULL != stats) {
            *stats = ops_stats;
        }
        //Очень тонкая работа с битами!
        if (ops & DUEL_OP_WRITING) {
            //Сырая запись отменяет консистентность
            if (ops & DUEL_OP_RAW_WRITING) {
                ops_stats &= ~DUEL_STAT_STR_CONSISTENCY;
            }
            //Строковая запись гарантирует консистентность.
            else {
                ops_stats |= DUEL_STAT_STR_CONSISTENCY;
            }
        }
        //Строковое чтение гарантирует консистентность
        else if (ops & DUEL_OP_STR_READING) {
            ops_stats |= DUEL_STAT_STR_CONSISTENCY;
        }
    }

    spin_unlock(&ops_spinlock);
    return result;
}

void duel_restore_ops(unsigned long ops) {
    spin_lock(&ops_spinlock);
    ops_access = ops_access ^ ops;
    spin_unlock(&ops_spinlock);
}