#ifndef _DUEL_STR_DEVICE_H_
#define _DUEL_STR_DEVICE_H_

#include <linux/cdev.h>

struct duel_str_dev {
    struct cdev cdev;
};

struct duel_str_filp_data {
    struct duel_str_dev* device;
    u8 access;
};

//Устанавливает NULL в случае неудачи.
extern int duel_alloc_str_dev(struct duel_str_dev** device, int major, int minor);

//Может безопасно получать NULL.
extern void duel_free_str_dev(struct duel_str_dev* device);

#endif