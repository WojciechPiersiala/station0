#pragma once
#define I2S_DAT_PIN 34
#define I2S_WS_PIN 26
// #define I2S_WS_PIN I2S_GPIO_UNUSED
#define I2S_CLK_PIN 0
#define BUFF_SIZE 64



void init_i2s();
void i2s_read_task(void *args);