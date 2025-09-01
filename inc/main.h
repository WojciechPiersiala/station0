#pragma once


/* wifi config */
#define WIFI_NETWORK 1
#if WIFI_NETWORK == 1
    #define ESP_WIFI_SSID "INEA-54D4_2.4G"
    #define ESP_WIFI_PASS "KqkmRCph"
#elif WIFI_NETWORK == 2
    #define ESP_WIFI_SSID "TCL-5J3J-2.4GHz"
    #define ESP_WIFI_PASS "G6hgv9Tq396k"
#else
    #error "Invalid WIFI_NETWORK selected! Please define WIFI_NETWORK as 1 or 2."
#endif


/* module id */
#define MODULE_ID 12 //last octet of the IP address

/* tcp config */
#define DEFAULT_HOST_IP "192.168.1.10"           //tcp config  server IP
#define PORT 5050                               //tcp cofnig server port
#define TCP_STACK_SIZE 8192                     // tcp config stack size for tcp client task
#define TCP_HEADER_LEN 8                        //tcp header added to message
#define LOG_DATA 0                              // log all data samples

/* mic config*/
#define MIC_WAIT 64                             //  mic config  read wait time in ms
#define LOG_AUDIO 0                             // mic config log every audio transfer;
#define PDM_RX_FREQ_HZ 16000                   // mic config I2S PDM RX frequency
#define AUDIO_QUEUE_LENGTH 8                    // mic config holds 10 chunks max
#define BUFF_SIZE   (2048)                      //mic config audio buffer size