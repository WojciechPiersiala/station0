#ifndef MAIN_TIMER_H
#define MAIN_TIMER_H

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

void start_timer(TimerHandle_t *timer, TickType_t duration);
void stop_timer(TimerHandle_t *timer);

#endif // MAIN_TIMER_H