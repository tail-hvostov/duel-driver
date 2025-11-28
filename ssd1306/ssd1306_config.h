#ifndef _SSD1306_CONFIG_H_
#define _SSD1306_CONFIG_H_

#include <linux/types.h>
#include <linux/spi/spi.h>

struct ssd1306_config {
    u8 width;
    u8 height;
    u8 clk_div_ratio_and_osc_freq;
    u8 contrast;
    u8 vcomh;
    u8 col_start_addr;
    //Инициализруются динамически
    u8 pages;
    u16 graphics_buf_size;
};

extern int ssd1306_init_config(struct spi_device* spi);
extern void ssd1306_exit_config(struct spi_device* spi);

extern struct ssd1306_config* ssd1306_get_config(struct spi_device* spi);
extern u8 ssd1306_get_display_pages(struct ssd1306_config* config);
extern u16 ssd1306_get_graphics_buf_size(struct ssd1306_config* config);

#endif
