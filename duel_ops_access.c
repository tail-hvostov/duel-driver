#include "duel_ops_access.h"

#include <linux/spinlock.h>

static DEFINE_SPINLOCK(ops_spinlock);
static unsigned long ops_access = 0;
static unsigned long ops_stats = 0;
static unsigned int str_readers = 0;

int duel_request_ops(unsigned long ops, unsigned long* stats) {
    int result = 0;
    unsigned long new_ops_access;
    spin_lock(&ops_spinlock);

    new_ops_access = ops_access;
    if ((ops & DUEL_OP_WRITING) & ops_access) {
        result = -EPERM;
        goto ending;
    }
    new_ops_access |= (ops & DUEL_OP_WRITING);

    if ((ops & DUEL_OP_RAW_WRITING) && (str_readers > 0)) {
        result = -EPERM;
        goto ending;
    }
    new_ops_access |= (ops & DUEL_OP_RAW_WRITING);

    if ((ops & DUEL_OP_STR_READING) && (ops_access & DUEL_OP_RAW_WRITING)) {
        result = -EPERM;
        goto ending;
    }

    str_readers += (0 != (ops & DUEL_OP_STR_READING));
    ops_access = new_ops_access;

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
ending:
    spin_unlock(&ops_spinlock);
    return result;
}

void duel_restore_ops(unsigned long ops) {
    spin_lock(&ops_spinlock);
    str_readers -= (0 != (ops & DUEL_OP_STR_READING));
    ops_access = ops_access ^ (ops & (DUEL_OP_RAW_WRITING | DUEL_OP_WRITING));
    spin_unlock(&ops_spinlock);
}