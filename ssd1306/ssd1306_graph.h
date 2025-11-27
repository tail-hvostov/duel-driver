#ifndef _SSD1306_GRAPH_H_
#define _SSD1306_GRAPH_H_

#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>

#define SSD1306_DISPLAY_WIDTH 72
#define SSD1306_DISPLAY_PAGES 5

struct ssd1306_graph {
    struct gpio_desc* dc_gpio;
    u8* graphics_buf;
    struct spi_transfer graph_transfer;
    struct spi_message graph_message;
};

//Не выделяет память.
extern int ssd1306_init_graph(struct spi_device* spi);
extern void ssd1306_exit_graph(struct spi_device* spi);

extern u8* ssd1306_get_graphics_buf(struct spi_device* spi);
//Возвращает ненулевое значение в случае неудачи.
extern int ssd1306_redraw_pages(struct spi_device* spi, unsigned int first,
                                        unsigned int last);
extern int ssd1306_reset_graphics_buf(struct spi_device* spi);

#endif