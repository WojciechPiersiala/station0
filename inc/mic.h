#pragma once
#include "driver/i2s_pdm.h"


/* PDM */
#define BUFF_SIZE   2048
#define PDM_RX_CLK_IO   0
#define PDM_RX_DIN_IO   34
#define PDM_RX_FREQ_HZ          16000           // I2S PDM RX frequency


/* Autio queue */
#define AUDIO_QUEUE_ITEM_SIZE BUFF_SIZE  
#define AUDIO_QUEUE_LENGTH 10       // holds 10 chunks max


typedef struct {
    int16_t samples[AUDIO_QUEUE_ITEM_SIZE / 2];  // 1024 samples = 2048 bytes
    size_t length;  // number of bytes actually used
} AudioChunk;

extern QueueHandle_t audio_queue;
extern volatile bool startRecording;

/* PDM funcitons */
void mic_task(void *args);
// i2s_chan_handle_t mic_init_pdm_rx(void);


