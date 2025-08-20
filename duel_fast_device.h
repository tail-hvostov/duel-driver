#ifndef _DUEL_FAST_DEVICE_H_
#define _DUEL_FAST_DEVICE_H_

struct duel_fast_dev {
    struct cdev cdev;
};

//Устанавливает NULL в случае неудачи.
extern int duel_alloc_fast_dev(struct duel_fast_dev** device);

//Может безопасно получать NULL.
extern void duel_free_fast_dev(struct duel_fast_dev* device);

#endif