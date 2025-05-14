#pragma once


#define DEFAULT_HOST_IP "192.168.1.4"
#define PORT 5050

// #define FORMAT "utf-8"
#define DISCONNECT_MSG "!DISCONNECT"
#define HEADER 16
// #define PORT 8080
// #define TAG "tcp"


void tcp_client_task(void *pvParameters);
void send_with_header(char *payload, int socket);