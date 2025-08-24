#include "ssd1306_device.h"

#include <linux/delay.h>

#define SSD1306_DC_GPIO_GROUP "dc"
#define SSD1306_RES_GPIO_GROUP "res"

int ssd1306_init_device(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata;
    drvdata = kmalloc(sizeof(struct ssd1306_drvdata), GFP_KERNEL);
    if (!drvdata) {
        printk(KERN_WARNING "Duel: out of memory.\n");
        return -ENOMEM;
    }

    //devm_gpiod_get отличается тем, что он привязывает пин
    //к struct device, освобождая от необходимости вызывать devm_gpiod_put.
    //Вот до чего технологии дошли.
    drvdata->dc_gpio = devm_gpiod_get(&spi->dev, SSD1306_DC_GPIO_GROUP, GPIOD_OUT_LOW);
    drvdata->res_gpio = devm_gpiod_get(&spi->dev, SSD1306_RES_GPIO_GROUP, GPIOD_OUT_LOW);
    if (IS_ERR(drvdata->dc_gpio) || IS_ERR(drvdata->res_gpio)) {
        printk(KERN_WARNING "Duel: couldn't access ssd1306 pins.\n");
        kfree(drvdata);
        return -ENOENT;
    }
    mutex_init(&drvdata->mutex);

    spi_set_drvdata(spi, drvdata);
    return 0;
}

void ssd1306_free_device(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    mutex_destroy(&drvdata->mutex);
    kfree(drvdata);
}

inline int ssd1306_device_lock_interruptible(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    return mutex_lock_interruptible(&drvdata->mutex);
}

inline void ssd1306_device_unlock(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    mutex_unlock(&drvdata->mutex);
}

inline int ssd1306_device_trylock(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    return mutex_trylock(&drvdata->mutex);
}

inline void hard_reset(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    gpiod_set_value(drvdata->res_gpio, 1);
    fsleep(100000);
    gpiod_set_value(drvdata->res_gpio, 0);
    fsleep(100000);
}

int ssd1306_device_startup(struct spi_device* spi) {
    hard_reset(spi);
    return 0;
}

void ssd1306_device_exit(struct spi_device* spi) {

}