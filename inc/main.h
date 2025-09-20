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

/* Use separate cores and different priorites*/
//cores 
#define CORE_APP 0   // Wifi
#define CORE_MIC 1

// priorities
#define PRI_LOW     3
#define PRI_NORMAL  5
#define PRI_HIGH   10
#define PRI_RT     12




/* module id */
#define MODULE_ID 11 //last octet of the IP address

/* tcp config */
#define DEFAULT_HOST_IP "192.168.1.111"           //tcp config  server IP
#define PORT 5050                               //tcp cofnig server port
#define TCP_STACK_SIZE 12288 /*8192*/            // tcp config stack size for tcp client task and mic task    16384
#define TCP_HEADER_LEN 8                        //tcp header added to message
#define LOG_DATA 0                              // log all data samples

/* mic config*/
#define MIC_WAIT 64                             //  mic config  read wait time in ms
#define LOG_AUDIO 0                             // mic config log every audio transfer;

#define PDM_RX_FREQ_HZ (16000)                   // mic config I2S PDM RX frequency
#define AUDIO_QUEUE_LENGTH 2                    // mic config holds 10 chunks max
#define BUFF_SIZE   (6144)                      //mic config audio buffer size old: 4096 6144 8192
    

/* display config */
#define COLOR_GREEN 1
#define COLOR_RED 2
#define COLOR_BLUE 3

#if MODULE_ID == 11
    #define USE_COLOR COLOR_GREEN
#elif MODULE_ID == 12
    #define USE_COLOR COLOR_RED
#elif MODULE_ID == 13
    #define USE_COLOR COLOR_BLUE
#else
    #define USE_COLOR COLOR_BLACK
#endif



