#pragma once
#define DISCONNECT_MSG "!DISCONNECT"



extern volatile bool startTcp;

void run_tcp_client_task(void *pvParameters);
int send_with_header(int socket, const void *dataptr, size_t data_len, int flags, char headertyp, char* headerRest);
void try2connect_tcp_task();