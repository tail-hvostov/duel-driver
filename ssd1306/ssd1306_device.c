#include "ssd1306_device.h"
#include "../duel_debug.h"

#include <linux/delay.h>
#include <linux/byteorder/generic.h>

#define SSD1306_DC_GPIO_GROUP "dc"
#define SSD1306_RES_GPIO_GROUP "res"

void reset_conversation(struct ssd1306_drvdata* drvdata) {
    memset(&drvdata->transfers, 0, sizeof(struct spi_transfer) * SSD1306_TRANSFER_BUF_SIZE);
    drvdata->transfers[0].tx_buf = drvdata->cmd_buf;
    //Я посмотрел в исходниках, что init делает memset самостоятельно.
    spi_message_init(&drvdata->cmd_message);
    drvdata->cur_transfer = 0;
    drvdata->remaining_cmd_bytes = SSD1306_CMD_BUF_SIZE;
    spi_message_add_tail(&drvdata->transfers[0], &drvdata->cmd_message);
}

void shift_transfer(struct ssd1306_drvdata* drvdata) {
    struct spi_transfer* transfer;
    u8* buf = drvdata->cmd_buf + (SSD1306_CMD_BUF_SIZE - drvdata->remaining_cmd_bytes);
    drvdata->cur_transfer += 1;
    transfer = &drvdata->transfers[drvdata->cur_transfer];
    transfer->tx_buf = buf;
    spi_message_add_tail(transfer, &drvdata->cmd_message);
}

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
    reset_conversation(drvdata);

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

void order_u8(struct spi_device* spi, u8 command) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    struct spi_transfer* transfer;
    u8* ptr = drvdata->cmd_buf + (SSD1306_CMD_BUF_SIZE - drvdata->remaining_cmd_bytes);
    if ((drvdata->cur_transfer < SSD1306_TRANSFER_BUF_SIZE) &&
        (drvdata->remaining_cmd_bytes >= 1)) {
        transfer = &drvdata->transfers[drvdata->cur_transfer];
        *ptr = command;
        transfer->len += 1;
        drvdata->remaining_cmd_bytes -= 1;
    }
    #ifdef DUEL_DEBUG
    else {
        PDEBUG("Duel: order_u8 failed.\n");
    }
    #endif
}

void order_u16(struct spi_device* spi, u16 command) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    struct spi_transfer* transfer;
    u16* ptr = (u16*)(drvdata->cmd_buf + (SSD1306_CMD_BUF_SIZE - drvdata->remaining_cmd_bytes));
    if ((drvdata->cur_transfer < SSD1306_TRANSFER_BUF_SIZE) &&
        (drvdata->remaining_cmd_bytes >= 2)) {
        transfer = &drvdata->transfers[drvdata->cur_transfer];
        command = cpu_to_be16(command);
        *ptr = command;
        transfer->len += 2;
        drvdata->remaining_cmd_bytes -= 2;
    }
    #ifdef DUEL_DEBUG
    else {
        PDEBUG("Duel: order_u8 failed.\n");
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

int ssd1306_device_startup(struct spi_device* spi) {
    hard_reset(spi);
    //Set Display ON/OFF (AEh/AFh)
    //AEh выключает дисплей.
    order_u8(spi, 0xAE);
    //Старшие 4 бита аргумента устанавливают частоту осциллятора внутри
    //дисплея по некоторому правилу. Младшие устанавливают "делитель"
    //частоты, изменяющийся от 1 до 16.
    //Set Display Clock Divide Ratio/Oscillator Frequency
    order_u16(spi, 0xD5F0);
    //Set Multiplex Ratio
    //Контроллер SSD1306 поддерживает различное число строк.
    //Эта команда это число ограничивает. В моём случае это 39+1.
    order_u16(spi, 0xA827);
    //Set Display Offset
    //Вертикально смещает изображение на некоторое количество строк.
    order_u16(spi, 0xD300);
    //Set Display Start Line (0x40-0x7F)
    //Эта группа команд задаёт смещение строк дисплея в видеопамяти.
    order_u8(spi, 0x40);
    //set charge pump enable
    //Я не нашёл эту команду в даташите. DeepSeek сказал, что она управляет
    //повышением напряжения на внутреннем преобразователе. Значение 0x10
    //его отключает.
    order_u16(spi, 0x8D14);
    //Set Memory Addressing Mode
    //Устанавливает способ адресации. В моём случае - Page adressing mode.
    order_u16(spi, 0x2002);
    //Set Segment Re-map (A0h/A1h)
    //Команда A1h устанавливает обратный порядок нумерации столбцов.
    order_u8(spi, 0xA1);
    //Set COM Output Scan Direction (C0h/C8h)
    //C8h задаёт обратный порядок строк.
    //Работает даже с уже прорисованными данными.
    order_u8(spi, 0xC8);
    //Set COM Pins Hardware Configuration (DAh)
    //Тут сложно. Как я понял, это влияет на нумерацию строк (
    //шахматный порядок) и на их взаимное расположение.
    order_u16(spi, 0xDA12);
    //Функции нет в даташите контроллера, но есть в даташите моего китайца.
    //DeepSeek считает, что это как-то связано с выбором напряжения.
    order_u16(spi, 0xAD30);
    //Set Contrast Control for BANK0 (81h)
    //Устанавливает контрастность дисплея от 00h до FFh.
    order_u16(spi, 0x812F);
    //Set Pre-charge Period (D9h)
    //Устанавливает период заряда и разряда пикселей, кратный периоду CLK.
    order_u16(spi, 0xD922);
    //Set VCOMH Deselect Level (DBh)
    //Устанавливает напряжение VCOMH. Я даже не стал разбираться.
    order_u16(spi, 0xDB20);
    //Entire Display ON (A4h/A5h)
    //A4h - нормальный режим отображения пикселей.
    //A5h - весь экран заливается белым.
    order_u8(spi, 0xA4);
    //Set Normal/Inverse Display (A6h/A7h)
    //В нормальном режиме единица в видеопамяти означает белый пиксель.
    order_u8(spi, 0xA6);
    //Это две дружественные команды.
    //Set Lower Column Start Address for Page Addressing Mode (00h~0Fh)
    //Set Higher Column Start Address for Page Addressing Mode (10h~1Fh)
    //Младшие 4 бита каждой из команд задают какую-то из частей 8-битного
    //порядкового номера стартового столбца.
    order_u16(spi, 0x0C11);
    order_delay(spi, 100);
    order_u8(spi, 0xAF);
    order_delay(spi, 100);
    return send_commands(spi);
}

int ssd1306_device_exit(struct spi_device* spi) {
    order_u8(spi, 0xAE);
    order_u16(spi, 0x8D10);
    order_delay(spi, 150);
    return send_commands(spi);
}

inline u8* ssd1306_device_get_graphics_buf(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    return drvdata->graphics_buf;
}