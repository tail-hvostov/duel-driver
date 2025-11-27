#include "ssd1306_device.h"

int ssd1306_init_device(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata;
    int result;
    const struct ssd1306_config* default_config;

    drvdata = kmalloc(sizeof(struct ssd1306_drvdata), GFP_KERNEL);
    if (!drvdata) {
        printk(KERN_WARNING "Duel: out of memory.\n");
        return -ENOMEM;
    }
    spi_set_drvdata(spi, drvdata);
    result = ssd1306_init_cmd(spi);
    if (result) {
        kfree(drvdata);
        return result;
    }
    result = ssd1306_init_graph(spi);
    if (result) {
        ssd1306_exit_cmd(spi);
        kfree(drvdata);
        return result;
    }
    default_config = device_get_match_data(&spi->dev);
    if (!default_config) {
        ssd1306_exit_graph(spi);
        ssd1306_exit_cmd(spi);
        kfree(drvdata);
        return -ENODEV;
    }
    drvdata->config = *default_config;

    mutex_init(&drvdata->mutex);
    return 0;
}

void ssd1306_free_device(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    mutex_destroy(&drvdata->mutex);
    ssd1306_exit_graph(spi);
    ssd1306_exit_cmd(spi);
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

int ssd1306_device_startup(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    struct ssd1306_config* config = &drvdata->config;

    ssd1306_hard_reset(spi);
    //Set Display ON/OFF (AEh/AFh)
    //AEh выключает дисплей.
    ssd1306_order_u8(spi, 0xAE);
    //Старшие 4 бита аргумента устанавливают частоту осциллятора внутри
    //дисплея по некоторому правилу. Младшие устанавливают "делитель"
    //частоты, изменяющийся от 1 до 16.
    //Set Display Clock Divide Ratio/Oscillator Frequency
    ssd1306_order_u16(spi, 0xD500 | (u16)config->clk_div_ratio_and_osc_freq);
    //Set Multiplex Ratio
    //Контроллер SSD1306 поддерживает различное число строк.
    //Эта команда это число ограничивает. В моём случае это 39+1.
    ssd1306_order_u16(spi, 0xA800 | (u16)(config->height - 1));
    //Set Display Offset
    //Вертикально смещает изображение на некоторое количество строк.
    ssd1306_order_u16(spi, 0xD300);
    //Set Display Start Line (0x40-0x7F)
    //Эта группа команд задаёт смещение строк дисплея в видеопамяти.
    ssd1306_order_u8(spi, 0x40);
    //set charge pump enable
    //Я не нашёл эту команду в даташите. DeepSeek сказал, что она управляет
    //повышением напряжения на внутреннем преобразователе. Значение 0x10
    //его отключает.
    ssd1306_order_u16(spi, 0x8D14);
    //Set Memory Addressing Mode
    //Устанавливает способ адресации. В моём случае - Page adressing mode.
    ssd1306_order_u16(spi, 0x2002);
    //Set Segment Re-map (A0h/A1h)
    //Команда A1h устанавливает обратный порядок нумерации столбцов.
    ssd1306_order_u8(spi, 0xA1);
    //Set COM Output Scan Direction (C0h/C8h)
    //C8h задаёт обратный порядок строк.
    //Работает даже с уже прорисованными данными.
    ssd1306_order_u8(spi, 0xC8);
    //Set COM Pins Hardware Configuration (DAh)
    //Тут сложно. Как я понял, это влияет на нумерацию строк (
    //шахматный порядок) и на их взаимное расположение.
    ssd1306_order_u16(spi, 0xDA12);
    //Функции нет в даташите контроллера, но есть в даташите моего китайца.
    //DeepSeek считает, что это как-то связано с выбором напряжения.
    ssd1306_order_u16(spi, 0xAD30);
    //Set Contrast Control for BANK0 (81h)
    //Устанавливает контрастность дисплея от 00h до FFh.
    ssd1306_order_u16(spi, 0x8100 | (u16)config->contrast);
    //Set Pre-charge Period (D9h)
    //Устанавливает период заряда и разряда пикселей, кратный периоду CLK.
    ssd1306_order_u16(spi, 0xD922);
    //Set VCOMH Deselect Level (DBh)
    //Устанавливает напряжение VCOMH. Я даже не стал разбираться.
    ssd1306_order_u16(spi, 0xDB00 | (u16)config->vcomh);
    //Entire Display ON (A4h/A5h)
    //A4h - нормальный режим отображения пикселей.
    //A5h - весь экран заливается белым.
    ssd1306_order_u8(spi, 0xA4);
    //Set Normal/Inverse Display (A6h/A7h)
    //В нормальном режиме единица в видеопамяти означает белый пиксель.
    ssd1306_order_u8(spi, 0xA6);
    //Это две дружественные команды.
    //Set Lower Column Start Address for Page Addressing Mode (00h~0Fh)
    //Set Higher Column Start Address for Page Addressing Mode (10h~1Fh)
    //Младшие 4 бита каждой из команд задают какую-то из частей 8-битного
    //порядкового номера стартового столбца.
    ssd1306_order_u16(spi, 0x0010 | (((u16)(config->col_start_addr & 0xF0)) << 8)
        | (((u16)(config->col_start_addr & 0x0F)) >> 4));
    ssd1306_order_delay(spi, 100);
    ssd1306_order_u8(spi, 0xAF);
    ssd1306_order_delay(spi, 100);

    ssd1306_reset_graphics_buf(spi);
    return ssd1306_send_commands(spi);
}

int ssd1306_device_exit(struct spi_device* spi) {
    ssd1306_order_u8(spi, 0xAE);
    ssd1306_order_u16(spi, 0x8D10);
    ssd1306_order_delay(spi, 150);
    return ssd1306_send_commands(spi);
}

inline struct ssd1306_config* ssd1306_get_config(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    return &drvdata->config;
}

inline u8 ssd1306_get_display_pages(struct ssd1306_config* config) {
    return &config->height / 8;
}