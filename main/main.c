#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "tcp_client.h"
#include "mic.h"
#include "main.h"
#include "display.h"
#include "esp_pm.h"
#include <driver/gpio.h>
#include "battery.h"


volatile bool startTcp = true;

void app_main(void){
    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* prepare the audio queue*/
    audio_queue = xQueueCreate(AUDIO_QUEUE_LENGTH, sizeof(AudioChunk));
    if (audio_queue == NULL) {
        ESP_LOGE("main", "Failed to create audio queue!");
        return;
    }

    wifi_init_sta();
    
    /* start tasks */
    xTaskCreate(run_battery_task, "run_battery_task", 2048  , NULL, 5, NULL);
    xTaskCreate(mic_task, "mic_task", TCP_STACK_SIZE , NULL, 5, NULL);
    xTaskCreate(try2connect_tcp_task, "try2connect_tcp_task", 1024 , NULL, 5, NULL);
    xTaskCreate(run_display_task, "run_display_task", 4096 , NULL, 5, NULL);
}
