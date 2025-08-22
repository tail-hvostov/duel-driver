#ifndef _DUEL_SIMPLE_DEVICE_H_
#define _DUEL_SIMPLE_DEVICE_H_

#include <linux/cdev.h>

struct duel_simple_dev {
    struct cdev cdev;
};

struct duel_simple_filp_data {
    struct duel_simple_dev* device;
    unsigned long access;
};

//Устанавливает NULL в случае неудачи.
extern int duel_alloc_simple_dev(struct duel_simple_dev** device, int major, int minor);

//Может безопасно получать NULL.
extern void duel_free_simple_dev(struct duel_simple_dev* device);

#endif