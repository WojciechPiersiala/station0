#pragma once

#define sleepTime (1000/portTICK_PERIOD_MS)
#define HOST_IP "192.168.1.3"
#define PORT 8080

void tcp_client_task(void *pvParameters);