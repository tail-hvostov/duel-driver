#include "ssd1306_driver.h"

#include <linux/spi/spi.h>

static const struct of_device_id ssd1306_dt_ids[] = {
    { .compatible = "duel,ssd1306" }, // Совпадение с Device Tree
    { }
};

static struct spi_driver duel_ssd1306_driver = {
    .driver = {
        .name = SSD1306_DRIVER_NAME,
        .owner = THIS_MODULE,
        .of_match_table = ssd1306_dt_ids
    },
    //.probe = duel_ssd1306_probe,
    //.remove = duel_ssd1306_remove,
    //.shutdown = duel_ssd1306_shutdown
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

MODULE_DEVICE_TABLE(of, ssd1306_dt_ids);