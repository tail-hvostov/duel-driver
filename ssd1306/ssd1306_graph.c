#include "ssd1306_graph.h"
#include "ssd1306_device.h"
#include "ssd1306_cmd.h"

#define SSD1306_DC_GPIO_GROUP "dc"
#define SSD1306_PAGE_MASK 0x07

inline struct ssd1306_graph* get_graph(struct spi_device* spi) {
    struct ssd1306_drvdata* drvdata = spi_get_drvdata(spi);
    return &drvdata->graph;
}

int ssd1306_init_graph(struct spi_device* spi) {
    struct ssd1306_graph* graph = get_graph(spi);
    //devm_gpiod_get отличается тем, что он привязывает пин
    //к struct device, освобождая от необходимости вызывать devm_gpiod_put.
    //Вот до чего технологии дошли.
    graph->dc_gpio = devm_gpiod_get(&spi->dev, SSD1306_DC_GPIO_GROUP, GPIOD_OUT_LOW);
    if (IS_ERR(drvdata->dc_gpio)) {
        printk(KERN_WARNING "Duel: couldn't access the dc pin.\n");
        kfree(drvdata);
        return -ENOENT;
    }
    return 0;;
}

inline void ssd1306_exit_graph(struct spi_device* spi) {
    return;
}

inline u8* ssd1306_get_graphics_buf(struct spi_device* spi) {
    struct ssd1306_graph* graph = get_graph(spi);
    return graph->graphics_buf;
}

inline int select_page(struct spi_device* spi, unsigned int page) {
    u8 command = ((u8)page & SSD1306_PAGE_MASK) | (u8)0xB0;
    ssd1306_order_u8(spi, command);
    ssd1306_order_u16(spi, 0x0C11);
    return ssd1306_send_commands(spi);
}

inline void init_page_transfer(struct ssd1306_graph* graph, unsigned int page) {
    memset(&graph->graph_transfer, 0, sizeof(struct spi_transfer));
    graph->graph_transfer.tx_buf = graph->graphics_buf +
                                    page * SSD1306_DISPLAY_WIDTH;
    graph->graph_transfer.len = SSD1306_DISPLAY_WIDTH;
    spi_message_init(&graph->graph_message);
    spi_message_add_tail(&graph->graph_transfer, &graph->graph_message);
}

int redraw_page(struct spi_device* spi, unsigned int page) {
    int result;
    struct ssd1306_graph* graph = get_graph(spi);
    select_page(spi, page);
    init_page_transfer(graph, page);
    gpiod_set_value(graph->dc_gpio, 1);
    result = spi_sync(spi, &graph->graph_message);
    gpiod_set_value(graph->dc_gpio, 0);
    return 0;
}

inline int ssd1306_redraw_pages(struct spi_device* spi, unsigned int first,
                                        unsigned int last) {
    unsigned int i;
    int result = 0;
    for (i = first; i <= last; i++) {
        result += (redraw_page(spi, i) < 0);
    }
    return result;
}