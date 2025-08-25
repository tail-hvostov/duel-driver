#ifndef _SSD1306_DEVICE_H_
#define _SSD1306_DEVICE_H_

#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>
#include <linux/mutex.h>

struct ssd1306_drvdata {
    struct gpio_desc* dc_gpio;
    struct gpio_desc* res_gpio;
    //Мьютекс должен всегда использоваться внешними модулями.
    struct mutex mutex;
};

extern int ssd1306_init_device(struct spi_device* spi);
extern void ssd1306_free_device(struct spi_device* spi);

//Функции, упрощающие блокировки.
extern int ssd1306_device_lock_interruptible(struct spi_device* spi);
extern void ssd1306_device_unlock(struct spi_device* spi);
extern int ssd1306_device_trylock(struct spi_device* spi);

//Действия, рекомендованные производителем для совершения при
//начале работы с дисплеем. Использует шину SPI.
extern int ssd1306_device_startup(struct spi_device* spi);
extern int ssd1306_device_exit(struct spi_device* spi);

#endif