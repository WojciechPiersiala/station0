#include "esp_lcd_io_spi.h"
#include "esp_log.h"
#include "esp_err.h"
#include <esp_lcd_panel_dev.h>
#include <driver/spi_master.h>
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include <driver/gpio.h>
#include "esp_lcd_panel_io.h"
#include "display_text.h"
#include <main.h>
#include "display.h"



#define EXAMPLE_PIN_NUM_LCD_CS 5
#define EXAMPLE_PIN_NUM_SCLK 13
#define EXAMPLE_PIN_NUM_MOSI 15
#define EXAMPLE_PIN_NUM_LCD_DC 14
#define EXAMPLE_PIN_NUM_RST 12
#define PIN_BL_EN                27         

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ 10000000
#define EXAMPLE_LCD_CMD_BITS 8
#define EXAMPLE_LCD_PARAM_BITS 8

#define EXAMPLE_LCD_H_RES 135  
#define EXAMPLE_LCD_V_RES 240  

#define LCD_HOST SPI2_HOST 



static const char *tag = "display";

static void lcd_backlight_on(void) {
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << PIN_BL_EN,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io);
    gpio_set_level(PIN_BL_EN, 1); 
}


static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b){
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static void lcd_fill_color(esp_lcd_panel_handle_t panel, uint16_t color){
    const int lines = 16; 
    const int tile_px = EXAMPLE_LCD_H_RES * lines;
    uint16_t *tile = heap_caps_malloc(tile_px * sizeof(uint16_t), MALLOC_CAP_DMA);
    assert(tile);
    for (int i = 0; i < tile_px; ++i) tile[i] = color;

    for (int y = 0; y < EXAMPLE_LCD_V_RES; y += lines) {
        int y2 = y + lines;
        if (y2 > EXAMPLE_LCD_V_RES) y2 = EXAMPLE_LCD_V_RES;
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, 0, y, EXAMPLE_LCD_H_RES, y2, tile));
    }
    heap_caps_free(tile);
}

esp_lcd_panel_handle_t panel_handle = NULL;

void run_display_task(void *arg){
    /* Create an SPI bus */
    ESP_LOGI(tag, "Initialize SPI bus...");
    spi_bus_config_t buscfg = {
        .sclk_io_num = EXAMPLE_PIN_NUM_SCLK,
        .mosi_io_num = EXAMPLE_PIN_NUM_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * 20 * sizeof(uint16_t), 
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO)); 


    
    ESP_LOGI(tag, "Initialize LCD IO...");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_PIN_NUM_LCD_DC,
        .cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 1
    };
    
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    
    ESP_LOGI(tag, "Install LCD controller driver");
    
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    ESP_LOGI(tag, "Initialize LCD");
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    const uint8_t madctl = 0x08;    
    ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, 0x36, &madctl, 1));
    const uint8_t colmod = 0x55;     
    ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, 0x3A, &colmod, 1));

    
    esp_lcd_panel_set_gap(panel_handle, 52, 40);   
    lcd_backlight_on();
    
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    
    ESP_LOGI(tag, "Draw to LCD");

    lcd_fill_color(panel_handle, rgb565(255, 255, 255));

    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, true, false);
    
    char *text = malloc(50);
    sprintf(text, "%d.%d.%d.[%d]\n", 192, 168, 1, MODULE_ID);







    while(1){


        draw_text_5x7(panel_handle, 5, 20, "IP address:", rgb565(COLOR_OCT_1, COLOR_OCT_2, COLOR_OCT_3), rgb565(255,255,255), 2);
        draw_text_5x7(panel_handle, 5, 40, text, rgb565(COLOR_OCT_1, COLOR_OCT_2, COLOR_OCT_3), rgb565(255,255,255), 2);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    free(text);
}

