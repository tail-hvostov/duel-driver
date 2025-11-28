#include "ssd1306_config.h"
#include "ssd1306_device.h"

int ssd1306_init_config(struct spi_device* spi) {
    const struct ssd1306_config* default_config;
    struct ssd1306_config* config = ssd1306_get_config(spi);
    default_config = device_get_match_data(&spi->dev);
    if (!default_config) {
        return -ENODEV;
    }
    *config = *default_config;
    config->pages = config->height / 8 + (0 != (config->height % 8));
    config->graphics_buf_size = config->width = config->pages;
    return 0;
}

void ssd1306_exit_config(struct spi_device* spi) {}

inline struct ssd1306_config* ssd1306_get_config(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    return &drvdata->config;
}

inline u8 ssd1306_get_display_pages(struct ssd1306_config* config) {
    return config->pages;
}

u16 ssd1306_get_graphics_buf_size(struct ssd1306_config* config) {
    return config->graphics_buf_size;
}
