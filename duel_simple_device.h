#ifndef _DUEL_SIMPLE_DEVICE_H_
#define _DUEL_SIMPLE_DEVICE_H_

#include <linux/cdev.h>

struct duel_simple_dev {
    struct cdev cdev;
};

//Устанавливает NULL в случае неудачи.
extern int duel_alloc_simlple_dev(struct duel_simple_dev** device);

//Может безопасно получать NULL.
extern void duel_free_simple_dev(struct duel_simple_dev* device);

#endif