#include "ssd1306_device.h"
#include "../duel_debug.h"

#define SSD1306_DC_GPIO_GROUP "dc"
#define SSD1306_PAGE_MASK 0x07

int ssd1306_init_device(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata;
    int result;
    drvdata = kmalloc(sizeof(struct ssd1306_drvdata), GFP_KERNEL);
    if (!drvdata) {
        printk(KERN_WARNING "Duel: out of memory.\n");
        return -ENOMEM;
    }

    //devm_gpiod_get отличается тем, что он привязывает пин
    //к struct device, освобождая от необходимости вызывать devm_gpiod_put.
    //Вот до чего технологии дошли.
    drvdata->dc_gpio = devm_gpiod_get(&spi->dev, SSD1306_DC_GPIO_GROUP, GPIOD_OUT_LOW);
    if (IS_ERR(drvdata->dc_gpio)) {
        printk(KERN_WARNING "Duel: couldn't access the dc pin.\n");
        kfree(drvdata);
        return -ENOENT;
    }
    result = ssd1306_init_cmd(spi);
    if (result) {
        return result;
    }
    mutex_init(&drvdata->mutex);

    spi_set_drvdata(spi, drvdata);
    return 0;
}

void ssd1306_free_device(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    mutex_destroy(&drvdata->mutex);
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

inline int send_commands(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    int result;
    result = spi_sync(spi, &drvdata->cmd_message);
    reset_conversation(drvdata);
    return result;
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

inline int select_page(struct spi_device* spi, unsigned int page) {
    u8 command = ((u8)page & SSD1306_PAGE_MASK) | (u8)0xB0;
    order_u8(spi, command);
    order_u16(spi, 0x0C11);
    return send_commands(spi);
}

int redraw_page(struct spi_device* spi, unsigned int page) {
    select_page(spi, page);
    return 0;
}

inline int ssd1306_device_redraw_pages(struct spi_device* spi, unsigned int first,
                                        unsigned int last) {
    unsigned int i;
    int result = 0;
    for (i = first; i <= last; i++) {
        result += (redraw_page(spi, i) < 0);
    }
    return result;
}