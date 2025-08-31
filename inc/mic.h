#pragma once
#include "driver/i2s_pdm.h"
#include "main.h"

/* PDM */

#define PDM_RX_CLK_IO   0
#define PDM_RX_DIN_IO   34
// #define AUDIO_QUEUE_ITEM_SIZE BUFF_SIZE  



typedef struct {
    int16_t samples[BUFF_SIZE / 2];  // 1024 samples = 2048 bytes
    size_t length;  // number of bytes actually used
    int64_t timestamp;  // timestamp of the audio chunk
} AudioChunk;

extern QueueHandle_t audio_queue;
extern volatile bool startRecording;

/* PDM funcitons */
void mic_task(void *args);
// i2s_chan_handle_t mic_init_pdm_rx(void);


