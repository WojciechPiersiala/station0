#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "tcp_client.h"
#include "mic.h"

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

    /* wifi, tcp connection*/
    wifi_init_sta();
    // xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);

    /* microphone*/
    xTaskCreate(mic_task, "mic_task", 4096, NULL, 5, NULL);

    while(1){
        // ESP_LOGI("main", "startTcp: %d", startTcp);
        if(startTcp){
            ESP_LOGI("main", "Restarting TCP client task ...");
            vTaskDelay(pdMS_TO_TICKS(5000));
            xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
            startTcp = false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
