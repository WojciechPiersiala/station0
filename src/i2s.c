#include "i2s.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include <stdlib.h>

static const char *TAG = "I2S_MIC";
static i2s_chan_handle_t rx_chan;

void i2s_mic_init(void) {
    ESP_LOGI(TAG, "Initializing I2S microphone...");

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_chan));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_24BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = I2S_STD_SLOT_LEFT,
            .ws_width = I2S_SLOT_BIT_WIDTH_32BIT,
            .bit_shift = false,
            .msb_right = false,
        },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = 18,
            .ws   = 19,
            .dout = I2S_GPIO_UNUSED,
            .din  = 34,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
}


void i2s_mic_task(void *args) {
    uint8_t *buf = calloc(1, I2S_BUFFER_SIZE);
    if (!buf) {
        ESP_LOGE(TAG, "Buffer allocation failed");
        vTaskDelete(NULL);
    }

    size_t bytes_read = 0;
    while (1) {
        if (i2s_channel_read(rx_chan, buf, I2S_BUFFER_SIZE, &bytes_read, pdMS_TO_TICKS(100)) == ESP_OK) {
            ESP_LOGI(TAG, "Read %d bytes", (int)bytes_read);
            for (int i = 0; i < 8 && i < bytes_read; i++) {
                printf("[%d] 0x%02X ", i, buf[i]);
            }
            printf("\n\n");
        } else {
            ESP_LOGW(TAG, "I2S read timeout or error");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    free(buf);
    vTaskDelete(NULL);
}
