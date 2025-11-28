#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "driver/i2s_pdm.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "sdkconfig.h"
#include "mic.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "main.h"
#include "esp_timer.h"

QueueHandle_t audio_queue;
volatile bool startRecording = false;
static const char* tag = "mic";
static int buffFullCount = 0;
static int buffBackFill = 30;
int64_t synchOffsetUs = 0;
int64_t messageNum = 0;

i2s_chan_handle_t mic_init_pdm_rx(void)
{
    i2s_chan_handle_t rx_chan;  
    
    ESP_LOGI(tag, "Adding new channel ...");
    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_cfg, NULL, &rx_chan));

    
    ESP_LOGI(tag, "Preparing PDM configuration ...");
    i2s_pdm_rx_config_t pdm_rx_cfg = {
        .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(PDM_RX_FREQ_HZ),
    
        .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .clk = PDM_RX_CLK_IO,
            .din = PDM_RX_DIN_IO,
            .invert_flags = {
                .clk_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_pdm_rx_mode(rx_chan, &pdm_rx_cfg));

    
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
    return rx_chan;
}


void mic_task(void *args)
{
    startRecording = false;
    i2s_chan_handle_t rx_chan = mic_init_pdm_rx();

    AudioChunk chunk;
    
    xQueueReset(audio_queue); 
    int micFailedCount = 0;
    while (1) {
        if(startRecording){

            esp_err_t err = i2s_channel_read(rx_chan, chunk.samples, sizeof(chunk.samples), &chunk.length, portMAX_DELAY);
            int64_t t1 = esp_timer_get_time();

            size_t  samples = chunk.length / sizeof(chunk.samples[0]);
            double  dur_us  = (double)samples * (1e6 / (double)PDM_RX_FREQ_HZ);
            // int64_t t_duration_us = chunk.length / PDM_RX_FREQ_HZ * 1e6;
            chunk.timestamp = (int64_t)((double)t1 - dur_us) + synchOffsetUs;
            chunk.messageNum = messageNum;
            messageNum++;

            #if LOG_AUDIO
                ESP_LOGI("mic", "read %u samples in %lld us -> %.2f kHz", (unsigned)samples, (long long)(dur_us), 1e3 * samples / (double)(dur_us));
                ESP_LOGI(tag, "Audio chunk timestamp: %lld", chunk.timestamp);
                
                chunk.read_time = t1 - t0; 
            #endif
            
            

            if(err == ESP_OK){
                
                if(chunk.length != sizeof(chunk.samples)){
                    ESP_LOGW(tag, "Partial chunk received: %d bytes", chunk.length);
                    continue; 
                }
                
                
                if (xQueueSend(audio_queue, &chunk, pdMS_TO_TICKS(100)) != pdPASS) {
                    
                    ESP_LOGW(tag, "Audio queue full, dropping full frame");
                    buffFullCount++;

                    if(buffFullCount > buffBackFill){
                        ESP_LOGE(tag, "Audio queue consistently full, Restarting the module ...");
                        esp_restart();
                    }
                }

            }
            else{
                ESP_LOGW(tag, "Failed to read data from I2S, Failure: %d", micFailedCount);
                micFailedCount++;
                if(micFailedCount > 10) {    
                    ESP_LOGE(tag, "Too many I2S read failures, Restarting the module ...");
                    esp_restart(); 
                }
            }
        }
        else{
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    vTaskDelete(NULL);
}
