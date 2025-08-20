#include "duel_fast_device.h"

//Устанавливает NULL в случае неудачи.
int duel_alloc_fast_dev(struct duel_fast_dev** device) {
    struct duel_fast_dev* instance;
    instance = kmalloc(sizeof(struct duel_fast_dev), GFP_KERNEL);
    if (!instance) {
        printk(KERN_WARNING "Duel: out of memory.\n");
        *device = NULL;
        return -ENOMEM;
    }
    *device = instance;
    return 0;
}

//Может безопасно получать NULL.
void duel_free_fast_dev(struct duel_fast_dev* device) {
    if (device != NULL) {
        kfree(device);
    }
}