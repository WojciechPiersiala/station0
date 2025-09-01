#include "battery.h"
#include <driver/gpio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *tag = "battery";
void run_battery_task(void *arg){
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << GPIO_NUM_4,   // HOLDGPIO4
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io);
    gpio_set_level(GPIO_NUM_4, 1);          // assert HOLD -> stay on battery
    ESP_LOGI(tag, "Battery task started");


    gpio_config_t io2 = {
        .pin_bit_mask = 1ULL << GPIO_NUM_39,   // HOLDGPIO4
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io2);
    ESP_LOGI(tag, "Button configured on GPIO39");

    while(1){
        int poweroffButton = gpio_get_level(GPIO_NUM_39);
        vTaskDelay(pdMS_TO_TICKS(1000));
        if(!poweroffButton){
            ESP_LOGI(tag, "Poweroff button pressed, shutting down ...");
            gpio_set_level(GPIO_NUM_4, 0);   // deassert HOLD -> power off
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        // ESP_LOGI(tag, "Button: %d", level);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}