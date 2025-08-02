/*#include <linux/module.h>
#include <linux/spi/spi.h>

static struct spi_driver duel_spi_driver = {

};

module_spi_driver(duel_spi_driver);

MODULE_LICENSE("Dual BSD/GPL");*/

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>

#define DRIVER_NAME "spi_example"

struct spi_device *spi_device;

static int spi_example_probe(struct spi_device *spi)
{
    int ret;
    
    printk(KERN_INFO "SPI Example Driver: Probe\n");
    
    // Сохраняем SPI устройство
    spi_device = spi;
    
    // Настройка SPI
    spi->mode = SPI_MODE_0;  // Режим SPI
    spi->bits_per_word = 8;  // 8 бит на слово
    spi->max_speed_hz = 1000000;  // 1 MHz
    
    ret = spi_setup(spi);
    if (ret < 0) {
        printk(KERN_ERR "Failed to setup SPI\n");
        return ret;
    }
    
    return 0;
}

static int spi_example_remove(struct spi_device *spi)
{
    printk(KERN_INFO "SPI Example Driver: Remove\n");
    return 0;
}

// Функция для записи данных
static int spi_write_data(u8 *data, size_t len)
{
    int ret;
    struct spi_transfer t = {
        .tx_buf = data,
        .len = len,
    };
    struct spi_message m;
    
    spi_message_init(&m);
    spi_message_add_tail(&t, &m);
    ret = spi_sync(spi_device, &m);
    
    return ret;
}

// Функция для чтения данных
static int spi_read_data(u8 *data, size_t len)
{
    int ret;
    struct spi_transfer t = {
        .rx_buf = data,
        .len = len,
    };
    struct spi_message m;
    
    spi_message_init(&m);
    spi_message_add_tail(&t, &m);
    ret = spi_sync(spi_device, &m);
    
    return ret;
}

static const struct of_device_id spi_example_dt_ids[] = {
    { .compatible = "example,spi-device" },
    { }
};
MODULE_DEVICE_TABLE(of, spi_example_dt_ids);

static struct spi_driver spi_example_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(spi_example_dt_ids),
    },
    .probe = spi_example_probe,
    .remove = spi_example_remove,
};

module_spi_driver(spi_example_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple SPI Device Driver for OrangePi Zero 3");