#include <linux/module.h>
#include <linux/spi/spi.h>

static struct spi_driver duel_spi_driver = {

};

module_spi_driver(duel_spi_driver);

MODULE_LICENSE("Dual BSD/GPL");