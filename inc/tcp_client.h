#pragma once




#define DISCONNECT_MSG "!DISCONNECT"
#define HEADER 16
#define PAYLOAD_LEN 256

extern volatile bool startTcp;


void tcp_client_task(void *pvParameters);
void send_with_header(char *payload, int socket);