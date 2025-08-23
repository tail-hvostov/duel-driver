#include "ssd1306_device.h"

int ssd1306_init_device(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata;
    drvdata = kmalloc(sizeof(struct ssd1306_drvdata), GFP_KERNEL);
    if (!drvdata) {
        printk(KERN_WARNING "Duel: out of memory.\n");
        return -ENOMEM;
    }
    spi_set_drvdata(spi, drvdata);
    return 0;
}

void ssd1306_free_device(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    kfree(drvdata);
}