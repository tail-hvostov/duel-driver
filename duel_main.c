#include <linux/init.h>
#include <linux/module.h>

static void duel_exit(void) {
}

static int __init duel_init(void) {
    return 0;
}

module_init(duel_init);
module_exit(duel_exit);

MODULE_LICENSE("Dual BSD/GPL");