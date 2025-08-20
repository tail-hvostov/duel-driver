#include "duel_simple_device.h"

//Устанавливает NULL в случае неудачи.
int duel_alloc_simlple_dev(struct duel_fast_dev** device) {
    return 0;
}

//Может безопасно получать NULL.
void duel_free_simple_dev(struct duel_fast_dev* device) {
    if (device != NULL) {
        
    }
}