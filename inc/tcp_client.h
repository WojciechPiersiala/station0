#pragma once

#define DEFAULT_HOST_IP "192.168.1.8"
#define PORT 5050


#define DISCONNECT_MSG "!DISCONNECT"
#define HEADER 16
#define PAYLOAD_LEN 256

extern volatile bool startTcp;


void tcp_client_task(void *pvParameters);
void send_with_header(char *payload, int socket);