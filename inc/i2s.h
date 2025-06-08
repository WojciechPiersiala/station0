#pragma once

#define I2S_MIC_DATA_PIN     34
#define I2S_MIC_CLK_PIN      0
#define I2S_MIC_WS_PIN       26
#define I2S_SAMPLE_RATE      16000
#define I2S_BUFFER_SIZE      512

void i2s_mic_init(void);
void i2s_mic_task(void *args);
