#include "tcp_client.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_log.h"

static const uint32_t sleepTime = 1000/portTICK_PERIOD_MS;

void tcp_client_task(void *pvParameters)
{
    char *host_ip = HOST_IP;
    int port = PORT;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(host_ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE("TCP", "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI("TCP", "Connecting to %s:%d", host_ip, port);
    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE("TCP", "Socket unable to connect: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI("TCP", "Successfully connected");

    char payload[] = "Hello from ESP32!";
    while(1){
        send(sock, payload, strlen(payload), 0);
        ESP_LOGI("TCP", "Message sent");
        vTaskDelay(sleepTime);
    }
    

    close(sock);
    vTaskDelete(NULL);
}