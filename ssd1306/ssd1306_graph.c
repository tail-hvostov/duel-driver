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
    struct ssd1306_config* config = ssd1306_get_config(spi);
    //devm_gpiod_get отличается тем, что он привязывает пин
    //к struct device, освобождая от необходимости вызывать devm_gpiod_put.
    //Вот до чего технологии дошли.
    graph->dc_gpio = devm_gpiod_get(&spi->dev, SSD1306_DC_GPIO_GROUP, GPIOD_OUT_LOW);
    if (IS_ERR(graph->dc_gpio)) {
        printk(KERN_WARNING "Duel: couldn't access the dc pin.\n");
        return -ENOENT;
    }
    graph->graphics_buf = kmalloc(ssd1306_get_graphics_buf_size(config), GFP_KERNEL);
    if (!graph->graphics_buf) {
        printk(KERN_WARNING "Duel: out of memory.\n");
        return -ENOMEM;
    }
    return 0;
}

inline void ssd1306_exit_graph(struct spi_device* spi) {
    struct ssd1306_graph* graph = get_graph(spi);
    kfree(graph->graphics_buf);
    return;
}

inline u8* ssd1306_get_graphics_buf(struct spi_device* spi) {
    struct ssd1306_graph* graph = get_graph(spi);
    return graph->graphics_buf;
}

inline int select_page(struct spi_device* spi, unsigned int page) {
    struct ssd1306_config* config = ssd1306_get_config(spi);
    u8 command = ((u8)page & SSD1306_PAGE_MASK) | (u8)0xB0;
    ssd1306_order_u8(spi, command);
    ssd1306_order_u16(spi, 0x0010 | (((u16)(config->col_start_addr & 0xF0)) >> 4)
        | (((u16)(config->col_start_addr & 0x0F)) << 8));
    return ssd1306_send_commands(spi);
}

inline void init_page_transfer(struct spi_device* spi, unsigned int page) {
    struct ssd1306_graph* graph = get_graph(spi);
    struct ssd1306_config* config = ssd1306_get_config(spi);
    memset(&graph->graph_transfer, 0, sizeof(struct spi_transfer));
    graph->graph_transfer.tx_buf = graph->graphics_buf +
                                    page * config->width;
    graph->graph_transfer.len = config->width;
    spi_message_init(&graph->graph_message);
    spi_message_add_tail(&graph->graph_transfer, &graph->graph_message);
}

int redraw_page(struct spi_device* spi, unsigned int page) {
    int result;
    struct ssd1306_graph* graph = get_graph(spi);
    select_page(spi, page);
    init_page_transfer(spi, page);
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

inline int ssd1306_reset_graphics_buf(struct spi_device* spi) {
    struct ssd1306_graph* graph = get_graph(spi);
    struct ssd1306_config* config = ssd1306_get_config(spi);
    memset(graph->graphics_buf, 0, ssd1306_get_graphics_buf_size(config));
    return ssd1306_redraw_pages(spi, 0, ssd1306_get_display_pages(config) - 1);
}
