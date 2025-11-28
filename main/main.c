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
    


    xTaskCreatePinnedToCore(run_battery_task, "run_battery_task", 2048, NULL, PRI_LOW, NULL, CORE_APP);
    xTaskCreatePinnedToCore(mic_task, "mic_task", 8192, NULL, PRI_RT, NULL, CORE_MIC);
    xTaskCreatePinnedToCore(try2connect_tcp_task, "try2connect_tcp_task", TCP_STACK_SIZE, NULL, PRI_HIGH, NULL, CORE_APP);
    xTaskCreatePinnedToCore(run_display_task, "run_display_task", 4096, NULL, PRI_NORMAL, NULL, CORE_APP);
    
}
