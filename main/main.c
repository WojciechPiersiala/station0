#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "tcp_client.h"
#include "mic.h"


void app_main(void){
    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* wifi, tcp connection*/
    #if 1
        wifi_init_sta();
        xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
    #endif
    /* microphone*/
    #if 1
        xTaskCreate(mic_task, "mic_task", 4096, NULL, 5, NULL);
    #endif

}
