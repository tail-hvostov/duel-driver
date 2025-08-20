#include "duel_fast_device.h"

//Устанавливает NULL в случае неудачи.
int duel_alloc_fast_dev(struct duel_fast_dev** device) {
    return 0;
}

//Может безопасно получать NULL.
void duel_free_fast_dev(struct duel_fast_dev* device) {
    if (device != NULL) {
        
    }
}