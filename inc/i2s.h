#pragma once
#define I2S_DAT_PIN 34
#define I2S_CLK_PIN 0
#define BUFF_SIZE 1024


void init_i2s();
static void i2s_read_task(void *args);