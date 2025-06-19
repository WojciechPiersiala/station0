#pragma once
#include "driver/i2s_pdm.h"


#define BUFF_SIZE   2048

#define PDM_RX_CLK_IO   0
#define PDM_RX_DIN_IO   34

#define PDM_RX_FREQ_HZ          16000           // I2S PDM RX frequency

void i2s_example_pdm_rx_task(void *args);
static i2s_chan_handle_t i2s_example_init_pdm_rx(void);


