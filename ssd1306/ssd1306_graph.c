#include "ssd1306_graph.h"

inline u8* ssd1306_device_get_graphics_buf(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    return drvdata->graphics_buf;
}

inline int select_page(struct spi_device* spi, unsigned int page) {
    u8 command = ((u8)page & SSD1306_PAGE_MASK) | (u8)0xB0;
    ssd1306_order_u8(spi, command);
    ssd1306_order_u16(spi, 0x0C11);
    return ssd1306_send_commands(spi);
}

inline void init_page_transfer(struct ssd1306_drvdata* drvdata, unsigned int page) {
    memset(&drvdata->graph_transfer, 0, sizeof(struct spi_transfer));
    drvdata->graph_transfer.tx_buf = drvdata->graphics_buf +
                                        page * SSD1306_DISPLAY_WIDTH;
    drvdata->graph_transfer.len = SSD1306_DISPLAY_WIDTH;
    spi_message_init(&drvdata->graph_message);
    spi_message_add_tail(&drvdata->graph_transfer, &drvdata->graph_message);
}

int redraw_page(struct spi_device* spi, unsigned int page) {
    int result;
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    select_page(spi, page);
    init_page_transfer(drvdata, page);
    gpiod_set_value(drvdata->dc_gpio, 1);
    result = spi_sync(spi, &drvdata->graph_message);
    gpiod_set_value(drvdata->dc_gpio, 0);
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