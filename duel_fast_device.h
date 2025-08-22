#ifndef _DUEL_FAST_DEVICE_H_
#define _DUEL_FAST_DEVICE_H_

#include <linux/cdev.h>

struct duel_fast_dev {
    struct cdev cdev;
};

struct duel_fast_filp_data {
    struct duel_fast_dev* device;
    unsigned long access;
};

//Устанавливает NULL в случае неудачи.
extern int duel_alloc_fast_dev(struct duel_fast_dev** device, int major, int minor);

//Может безопасно получать NULL.
extern void duel_free_fast_dev(struct duel_fast_dev* device);

#endif