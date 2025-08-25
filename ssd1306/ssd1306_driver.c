#include "ssd1306_driver.h"
#include "ssd1306_device.h"

static struct spi_device* device_instance = NULL;

static const struct of_device_id ssd1306_dt_ids[] = {
    { .compatible = "duel,ssd1306" }, // Совпадение с Device Tree
    { }
};

//Вызывается при выключении компьютера.
static void shutdown_device(struct spi_device *spi) {
    ssd1306_device_exit(spi);
}

//Вызывается при отсоединении устройства от драйвера в нормальных условиях.
static void remove_device(struct spi_device *spi) {
    //Я здесь пользуюсь тем, что remove может быть вызван только в случае
    //успешного rmmod.
    if (ssd1306_device_exit(spi)) {
        printk(KERN_WARNING "Duel: couldn't exit the display.\n");
    }
    ssd1306_free_device(spi);
}

//Вызывается при подключении устройства к драйверу.
static int probe_device(struct spi_device* spi) {
    int result;
    result = ssd1306_init_device(spi);
    if (result) {
        return result;
    }
    //Пока доступа ни у кого нет, можно не блокировать.
    if (ssd1306_device_startup(spi)) {
        printk(KERN_WARNING "Duel: couldn't init the display.\n");
    }
    if (!device_instance) {
        device_instance = spi;
    }
    return 0;
}

static struct spi_driver duel_ssd1306_driver = {
    .driver = {
        .name = SSD1306_DRIVER_NAME,
        .owner = THIS_MODULE,
        .of_match_table = ssd1306_dt_ids
    },
    .probe = probe_device,
    .remove = remove_device,
    .shutdown = shutdown_device
};

int ssd1306_init_driver(void) {
    int result = 0;
    //Регистрация драйвера SPI.
    result = spi_register_driver(&duel_ssd1306_driver);
    if (result) {
        printk(KERN_WARNING "Duel: couldn't register the spi_driver.\n");
    }
    return result;
}

void ssd1306_exit_driver(void) {
    spi_unregister_driver(&duel_ssd1306_driver);
}

inline struct spi_device* ssd1306_get_spi_device(void) {
    return device_instance;
}

MODULE_DEVICE_TABLE(of, ssd1306_dt_ids);