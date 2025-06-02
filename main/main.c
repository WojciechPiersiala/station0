#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "tcp_client.h"
#include "i2s.h"

// uint32_t sleepTime = 1/portTICK_PERIOD_MS;




// void taskMic(void *a){
//     init_i2s();
//     while(1){

//     }
// }
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
    // vTaskDelay(sleepTime);

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

    /* wifi, tcp connection*/
    wifi_init_sta();
    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);

    /* microphone*/
    init_i2s();
    xTaskCreate(i2s_read_task, "i2s_example_read_task", 4096, NULL, 5, NULL);

    // gpio_set_direction(2, GPIO_MODE_OUTPUT);
    // xTaskCreate(taskMic, "Mic", 2048, NULL, 0, NULL); //task, debugName, size, args, priority
}
