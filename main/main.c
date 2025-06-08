#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "tcp_client.h"
#include "i2s.h"


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
    #if 0 // dont use wifi
    wifi_init_sta();
    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
    #endif
    /* microphone*/
    i2s_mic_init();
    xTaskCreate(i2s_mic_task, "i2s_example_read_task", 4096, NULL, 5, NULL);

    // gpio_set_direction(2, GPIO_MODE_OUTPUT);
    // xTaskCreate(taskMic, "Mic", 2048, NULL, 0, NULL); //task, debugName, size, args, priority
}
