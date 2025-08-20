#include "duel_str_device.h"

//Устанавливает NULL в случае неудачи.
int duel_alloc_str_dev(struct duel_str_dev** device) {
    return 0;
}

//Может безопасно получать NULL.
void duel_free_str_dev(struct duel_str_dev* device) {
    if (device != NULL) {

    }
}