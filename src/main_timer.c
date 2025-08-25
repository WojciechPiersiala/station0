#include "main_timer.h"


void my_timer_callback(TimerHandle_t xTimer) {
    ESP_LOGI("main", "Timer expired!");
    // Do something here
}

void start_timer(TimerHandle_t *timer, TickType_t duration){
    TimerHandle_t my_timer = xTimerCreate(
        "MyTimer",                      // Timer name
        pdMS_TO_TICKS(2000),            // Timer period in ticks
        pdTRUE,                         // Auto-reload (periodic)
        NULL,                           // Timer ID (optional)
        my_timer_callback               // Callback function
    );

    if (my_timer != NULL) {
        xTimerStart(my_timer, 0);       // Start the timer
    }
}
