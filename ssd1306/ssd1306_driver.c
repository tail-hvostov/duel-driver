#include "ssd1306_driver.h"

#include <linux/spi/spi.h>

static const struct of_device_id ssd1306_dt_ids[] = {
    { .compatible = "duel,ssd1306" }, // Совпадение с Device Tree
    { }
};

MODULE_DEVICE_TABLE(of, ssd1306_dt_ids);