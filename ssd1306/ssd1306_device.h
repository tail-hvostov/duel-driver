#ifndef _SSD1306_DEVICE_H_
#define _SSD1306_DEVICE_H_

#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>

struct ssd1306_drvdata {
    struct gpio_desc* dc_gpio;
    struct gpio_desc* res_gpio;
};

extern int ssd1306_init_device(struct spi_device* spi);
extern void ssd1306_free_device(struct spi_device* spi);

#endif