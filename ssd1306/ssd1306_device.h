#ifndef _SSD1306_DEVICE_H_
#define _SSD1306_DEVICE_H_

#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>
#include <linux/mutex.h>

#define SSD1306_CMD_BUF_SIZE 40
#define SSD1306_TRANSFER_BUF_SIZE 3

#define SSD1306_DISPLAY_WIDTH 72
#define SSD1306_DISPLAY_PAGES 5
#define SSD1306_GRAPHICS_BUF_SIZE (SSD1306_DISPLAY_WIDTH * SSD1306_DISPLAY_PAGES)

struct ssd1306_drvdata {
    struct gpio_desc* dc_gpio;
    struct gpio_desc* res_gpio;
    //Мьютекс должен всегда использоваться внешними модулями.
    struct mutex mutex;
    
    u8 cmd_buf[SSD1306_CMD_BUF_SIZE];
    struct spi_transfer transfers[SSD1306_TRANSFER_BUF_SIZE];
    int cur_transfer;
    int remaining_cmd_bytes;
    struct spi_message cmd_message;

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

#endif