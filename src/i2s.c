#include "i2s.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static i2s_chan_handle_t rx_chan;
static char *tag = "i2s";


void init_i2s(){
    ESP_LOGI(tag,"initialisation");

    ESP_LOGI(tag,"assign pin %d", I2S_DAT_PIN);
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);

    ESP_LOGI(tag,"create new channel");
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_chan));

    ESP_LOGI(tag,"create default configuration");
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(8000),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_CLK_PIN,
            .ws = I2S_WS_PIN,
            .dout = I2S_GPIO_UNUSED,
            .din = I2S_DAT_PIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_cfg));

    ESP_LOGI(tag,"init standard mode");
    ESP_LOGI(tag,"enable channel");

    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
    ESP_LOGI("I2S", "I2S microphone initialized");
}



void i2s_read_task(void *args)
{
    uint8_t *r_buf = (uint8_t *)calloc(1, BUFF_SIZE);
    assert(r_buf); // Check if r_buf allocation success
    size_t r_bytes = 0;

    // /* Enable the RX channel */
    // ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));

    /* ATTENTION: The print and delay in the read task only for monitoring the data by human,
     * Normally there shouldn't be any delays to ensure a short polling time,
     * Otherwise the dma buffer will overflow and lead to the data lost */
    while (1) {
        /* Read i2s data */
        if (i2s_channel_read(rx_chan, r_buf, BUFF_SIZE, &r_bytes, 1000) == ESP_OK) {
            printf("Read Task: i2s read %d bytes\n-----------------------------------\n", r_bytes);
            printf("[0] %x [1] %x [2] %x [3] %x [4] %x [5] %x [6] %x [7] %x\n\n",
                   r_buf[0], r_buf[1], r_buf[2], r_buf[3], r_buf[4], r_buf[5], r_buf[6], r_buf[7]);
        } else {
            ESP_LOGE(tag, "Read Task: i2s read failed\n");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    free(r_buf);
    vTaskDelete(NULL);
}