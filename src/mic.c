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



i2s_chan_handle_t mic_init_pdm_rx(void)
{
    i2s_chan_handle_t rx_chan;        // I2S rx channel handler
    /* Setp 1: Determine the I2S channel configuration and allocate RX channel only */
    ESP_LOGI(tag, "Adding new channel ...");
    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_cfg, NULL, &rx_chan));

    /* Step 2: Setting the configurations of PDM RX mode and initialize the RX channel */  
    ESP_LOGI(tag, "Preparing PDM configuration ...");
    i2s_pdm_rx_config_t pdm_rx_cfg = {
        .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(PDM_RX_FREQ_HZ),
        /* The data bit-width of PDM mode is fixed to 16 */
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

    /* Step 3: Enable the rx channels before reading data */
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
    return rx_chan;
}




void mic_task(void *args)
{
    startRecording = false;
    i2s_chan_handle_t rx_chan = mic_init_pdm_rx();

    AudioChunk chunk;
    
    xQueueReset(audio_queue); // clear the queue before starting recording
    while (1) {
        if(startRecording){
            /* Read i2s data */
            int64_t t0 = esp_timer_get_time();

            esp_err_t err = i2s_channel_read(rx_chan, chunk.samples, sizeof(chunk.samples), &chunk.length, portMAX_DELAY);
            #if LOG_AUDIO
                int64_t t1 = esp_timer_get_time();
            #endif
            chunk.timestamp = t0;


            if(err == ESP_OK){
                // warning if the audio data is not full
                if(chunk.length != sizeof(chunk.samples)){
                    ESP_LOGW(tag, "Partial chunk received: %d bytes", chunk.length);
                }
                
                // place the audio data in the queue
                int sendAudioRes = xQueueSend(audio_queue, &chunk, pdMS_TO_TICKS(10));
                if(sendAudioRes == pdPASS) {
                    #if LOG_AUDIO
                        ESP_LOGI(tag, "Sent audio chunk with %d bytes to the queue\n", chunk.length);
                        int64_t read_chunk_ms = (t1 - t0) / 1000;
                        ESP_LOGI(tag, "chunk period: %lld ms", read_chunk_ms);
                    #endif
                }
                else { // Audio data couldn't be placed in the queue
                    #if LOG_AUDIO
                        ESP_LOGW(tag, "Audio queue full, dropping frame");
                    #endif
                }

                // // place timestamp data in the queue
                // int sendTimeRes = 0;
                // if(sendAudioRes)
                //     sendTimeRes = xQueueSend(timestamp_queue, &t0, pdMS_TO_TICKS(10));
                // if(sendTimeRes == pdPASS) {
                //     #if LOG_AUDIO
                //         ESP_LOGI(tag, "Sent timestamp %lld to the queue\n", t0);
                     
                //         int64_t read_chunk_ms = (t1 - t0) / 1000;
                //         ESP_LOGI(tag, "chunk period: %lld ms", read_chunk_ms);
                //         ESP_LOGI(tag, "Sent timestamp %lld to the queue\n", t0);
                //     #endif 
                // }
                // else { // Timestamp data couldn't be placed in the queue
                //     #if LOG_AUDIO
                //         ESP_LOGW(tag, "Timestamp queue full, dropping frame");
                //     #endif
                // }
            }
            else{
                ESP_LOGE(tag, "Failed to read data from I2S");
            }
        }
        else{
            vTaskDelay(pdMS_TO_TICKS(10)); // delay the task when not recording
        }
    }
    // free(r_buf);
    vTaskDelete(NULL);
}
