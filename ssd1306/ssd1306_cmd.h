#ifndef _SSD1306_CMD_H_
#define _SSD1306_CMD_H_

#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>

#define SSD1306_CMD_BUF_SIZE 40
#define SSD1306_CMD_TRANSFER_BUF_SIZE 3

struct ssd1306_cmd {
    struct gpio_desc* res_gpio;
    u8 cmd_buf[SSD1306_CMD_BUF_SIZE];
    struct spi_transfer transfers[SSD1306_CMD_TRANSFER_BUF_SIZE];
    int cur_transfer;
    int remaining_cmd_bytes;
    struct spi_message cmd_message;
};

//Не выделяет память.
extern int ssd1306_init_cmd(struct spi_device* spi);
extern void ssd1306_exit_cmd(struct spi_device* spi);

extern void ssd1306_hard_reset(struct spi_device* spi);
extern void ssd1306_order_u8(struct spi_device* spi, u8 command);
extern void ssd1306_order_u16(struct spi_device* spi, u16 command);

#endif