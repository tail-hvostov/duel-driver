#ifndef _SSD1306_DEVICE_H_
#define _SSD1306_DEVICE_H_

#include <linux/mutex.h>

#include "ssd1306_cmd.h"

#define SSD1306_DISPLAY_WIDTH 72
#define SSD1306_DISPLAY_PAGES 5
#define SSD1306_GRAPHICS_BUF_SIZE (SSD1306_DISPLAY_WIDTH * SSD1306_DISPLAY_PAGES)

struct ssd1306_drvdata {
    struct gpio_desc* dc_gpio;
    //Мьютекс должен всегда использоваться внешними модулями.
    struct mutex mutex;
    struct ssd1306_cmd cmd;
    u8 graphics_buf[SSD1306_GRAPHICS_BUF_SIZE];
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

extern u8* ssd1306_device_get_graphics_buf(struct spi_device* spi);

//Возвращает ненулевое значение в случае неудачи.
extern int ssd1306_device_redraw_pages(struct spi_device* spi, unsigned int first,
                                        unsigned int last);

#endif