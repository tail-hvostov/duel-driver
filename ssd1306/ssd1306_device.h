#ifndef _SSD1306_DEVICE_H_
#define _SSD1306_DEVICE_H_

#include <linux/mutex.h>

#include "ssd1306_cmd.h"
#include "ssd1306_graph.h"

struct ssd1306_config {
    u8 width;
    u8 height;
    u8 clk_div_ratio_and_osc_freq;
    u8 contrast;
    u8 vcomh;
    u8 col_start_addr
};

struct ssd1306_drvdata {
    //Мьютекс должен всегда использоваться внешними модулями.
    struct mutex mutex;
    struct ssd1306_cmd cmd;
    struct ssd1306_graph graph;
    struct ssd1306_config config;
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

extern struct ssd1306_config* ssd1306_get_config(struct spi_device* spi);
extern u8 ssd1306_get_display_pages(struct ssd1306_config* config);

#endif