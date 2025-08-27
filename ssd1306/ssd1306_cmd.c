#include "ssd1306_cmd.h"
#include "ssd1306_device.h"
#include "../duel_debug.h"

#include <linux/delay.h>
#include <linux/byteorder/generic.h>

#define SSD1306_RES_GPIO_GROUP "res"

inline struct ssd1306_cmd* get_cmd(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    return &drvdata->cmd;
}

void reset_conversation(struct ssd1306_cmd* cmd) {
    memset(&cmd->transfers, 0, sizeof(struct spi_transfer) * SSD1306_CMD_TRANSFER_BUF_SIZE);
    cmd->transfers[0].tx_buf = cmd->cmd_buf;
    //Я посмотрел в исходниках, что init делает memset самостоятельно.
    spi_message_init(&cmd->cmd_message);
    cmd->cur_transfer = 0;
    cmd->remaining_cmd_bytes = SSD1306_CMD_BUF_SIZE;
    spi_message_add_tail(&cmd->transfers[0], &cmd->cmd_message);
}

int ssd1306_init_cmd(struct spi_device* spi) {
    struct ssd1306_cmd* cmd = get_cmd(spi);
    //devm_gpiod_get отличается тем, что он привязывает пин
    //к struct device, освобождая от необходимости вызывать devm_gpiod_put.
    //Вот до чего технологии дошли.
    cmd->res_gpio = devm_gpiod_get(&spi->dev, SSD1306_RES_GPIO_GROUP, GPIOD_OUT_LOW);
    if (IS_ERR(IS_ERR(cmd->res_gpio)) {
        printk(KERN_WARNING "Duel: couldn't access the res pin.\n");
        kfree(drvdata);
        return -ENOENT;
    }
    reset_conversation(cmd);
}

inline void ssd1306_exit_cmd(struct spi_device* spi) {
    return;
}

inline void ssd1306_hard_reset(struct spi_device* spi) {
    struct ssd1306_cmd* cmd = get_cmd(spi);
    gpiod_set_value(cmd->res_gpio, 1);
    fsleep(100000);
    gpiod_set_value(cmd->res_gpio, 0);
    fsleep(100000);
}

void ssd1306_order_u8(struct spi_device* spi, u8 command) {
    struct ssd1306_cmd* cmd = get_cmd(spi);
    struct spi_transfer* transfer;
    u8* ptr = cmd->cmd_buf + (SSD1306_CMD_BUF_SIZE - cmd->remaining_cmd_bytes);
    if ((cmd->cur_transfer < SSD1306_TRANSFER_BUF_SIZE) &&
        (cmd->remaining_cmd_bytes >= 1)) {
        transfer = &cmd->transfers[cmd->cur_transfer];
        *ptr = command;
        transfer->len += 1;
        cmd->remaining_cmd_bytes -= 1;
    }
    #ifdef DUEL_DEBUG
    else {
        PDEBUG("Duel: order_u8 failed.\n");
    }
    #endif
}

void ssd1306_order_u16(struct spi_device* spi, u16 command) {
    struct ssd1306_cmd* cmd = get_cmd(spi);
    struct spi_transfer* transfer;
    u16* ptr = (u16*)(cmd->cmd_buf + (SSD1306_CMD_BUF_SIZE - cmd->remaining_cmd_bytes));
    if ((cmd->cur_transfer < SSD1306_TRANSFER_BUF_SIZE) &&
        (cmd->remaining_cmd_bytes >= 2)) {
        transfer = &cmd->transfers[cmd->cur_transfer];
        command = cpu_to_be16(command);
        *ptr = command;
        transfer->len += 2;
        cmd->remaining_cmd_bytes -= 2;
    }
    #ifdef DUEL_DEBUG
    else {
        PDEBUG("Duel: order_u8 failed.\n");
    }
    #endif
}

void shift_transfer(struct ssd1306_drvdata* drvdata) {
    struct spi_transfer* transfer;
    u8* buf = drvdata->cmd_buf + (SSD1306_CMD_BUF_SIZE - drvdata->remaining_cmd_bytes);
    drvdata->cur_transfer += 1;
    transfer = &drvdata->transfers[drvdata->cur_transfer];
    transfer->tx_buf = buf;
    spi_message_add_tail(transfer, &drvdata->cmd_message);
}

void order_delay(struct spi_device* spi, unsigned millis) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    struct spi_transfer* transfer;
    if (drvdata->cur_transfer < SSD1306_TRANSFER_BUF_SIZE) {
        transfer = &drvdata->transfers[drvdata->cur_transfer];
        transfer->delay.value = millis * 1000;
        transfer->delay.unit = SPI_DELAY_UNIT_USECS;
        shift_transfer(drvdata);
    }
    #ifdef DUEL_DEBUG
    else {
        PDEBUG("Duel: transfer count exceeded.\n");
    }
    #endif
}

inline int send_commands(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    int result;
    result = spi_sync(spi, &drvdata->cmd_message);
    reset_conversation(drvdata);
    return result;
}