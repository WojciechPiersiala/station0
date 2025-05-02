#include "wifi.h"
#include "tcp_client.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_log.h"

static const uint32_t sleepTime = 1000/portTICK_PERIOD_MS;

void tcp_client_task(void *pvParameters){
    // char *host_ip = DEFAULT_HOST_IP;
    const char *host_ip = get_ip_str();
    int port = PORT;

    ESP_LOGI(TAG, "Debug IP = \t %s", host_ip);

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(host_ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Connecting to %s:%d", host_ip, port);
    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Successfully connected");

    char payload[] = "Hello from ESP32!";
    while(1){
        send(sock, payload, strlen(payload), 0);
        ESP_LOGI(TAG, "Message sent");
        vTaskDelay(sleepTime);
    }
    

    close(sock);
    vTaskDelete(NULL);
}