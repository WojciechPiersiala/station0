#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "tcp_client.h"


#include "driver/gpio.h"
#include "driver/i2s_std.h"

#define I2S_DAT_PIN 34
#define I2S_CLK_PIN 0
// uint32_t sleepTime = 1/portTICK_PERIOD_MS;



void init_i2s(){
    char *tag = "i2s";
    ESP_LOGI(tag,"initialisation");
    ESP_LOGI(tag,"assign pin %d", I2S_DAT_PIN);
    i2s_chan_handle_t rx_chan;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_LOGI(tag,"create new channel");
    i2s_new_channel(&chan_cfg, NULL, &rx_chan);

    ESP_LOGI(tag,"create default configuration");
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_GPIO_UNUSED,
            .ws = I2S_CLK_PIN,
            .dout = I2S_GPIO_UNUSED,
            .din = I2S_DAT_PIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    ESP_LOGI(tag,"init standard mode");
    i2s_channel_init_std_mode(rx_chan, &std_cfg);
    ESP_LOGI(tag,"enable channel");
    i2s_channel_enable(rx_chan);
    ESP_LOGI("I2S", "I2S microphone initialized");
}


void taskMic(void *a){
    init_i2s();
    while(1){

    }
}
    // //set up i2s processor
    // const i2s_config_t i2s_config =  {
    //     .mode = (I2S_MODE_MASTER | I2S_MODE_RX),
    //     .sample_rate = 44100,
    //     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    //     .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    //     .communication_format = I2S_MODE_PDM,
    //     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    // };
  
    // i2s_driver_install(I2S_NUM_AUTO, &i2s_config, 0, NULL);
    // // set up the pin
    // i2s_pin_config_t pin_config = {
    //     .mck_io_num = 
    //     .bck_io_num = 
    //     .ws_io_num = 
    //     .data_out_num = -1,
    //     .data_in_num = 
    // };

// I2S_NUM_0;

// gpio_set_direction(GPIO_NUM_34, GPIO_MODE_INPUT);

// char *tag = "GPIO";

// while(1){
//     // gpio_set_level(2, 34);
//     uint16_t val = gpio_get_level(134);
//     vTaskDelay(sleepTime);

//     // gpio_set_level(2, 1);
//     // vTaskDelay(sleepTime);
//     ESP_LOGI(tag, "GPIO34: %d", val);
// }


void app_main(void){
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("", "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);

    // gpio_set_direction(2, GPIO_MODE_OUTPUT);
    // xTaskCreate(taskMic, "Mic", 2048, NULL, 0, NULL); //task, debugName, size, args, priority
}
