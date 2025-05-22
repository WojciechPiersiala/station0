#include "wifi.h"
#include "tcp_client.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_log.h"

static const uint32_t sendSleepTime = 1000/portTICK_PERIOD_MS;
static const char* tag = "tcp";
static const int maxConn = 50;

static bool retryConnection = true;
static int connDone = 0;

void send_with_header(char *payload, int socket){
    // char payload[] = "10";
    // int payload_lenthg = sizeof(payload) / sizeof(payload[0]);
    uint payload_lenthg = strlen(payload);
    char payload_lenthg_msg[HEADER] = {0};
    snprintf(payload_lenthg_msg, HEADER, "%15d", payload_lenthg);
    // ESP_LOGI(tag, "Message length sent:%s", payload_lenthg_msg);
    // printf("%s \n",payload_lenthg_msg);
    // send(socket, payload_lenthg_msg, strlen(payload), 0);
    // char *payload2 = "9";
    send(socket, payload, HEADER, 0); 
    ESP_LOGI(tag, "Message lenght sent:%s", payload_lenthg_msg);
    send(socket, payload, strlen(payload), 0);
    ESP_LOGI(tag, "Message sent:%s", payload);
    // ESP_LOGI(tag, "Message sent:    %s", payload);
}


void tcp_client_task(void *pvParameters){
    // char *host_ip = DEFAULT_HOST_IP;
    // const char *host_ip = get_ip_str();
    const char *host_ip = DEFAULT_HOST_IP;
    int port = PORT;

    
    ESP_LOGI(tag, "socket connected to IP:  \t %s", host_ip);

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(host_ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(tag, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    else{
        ESP_LOGI(tag, "Socket created");
    }

    while(retryConnection){
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        ESP_LOGI(tag, "Connecting to %s:%d", host_ip, port);
        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGW(tag, "Socket unable to connect: errno %d, connection try: %d", errno, connDone);
            connDone++;
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(1000));
            // return;
        }
        else{
            ESP_LOGI(tag, "Successfully connected");
            retryConnection = false;
        }

        if(connDone > maxConn){
            ESP_LOGE(tag, "FAILURE: Socket unable to connect: errno %d", errno);
            retryConnection = false;
            close(sock);
            vTaskDelete(NULL);
            return;
        }
    }
    
    ESP_LOGI(tag, "Successfully connected");


     
    char payload[HEADER];
    for(int j=0; j<10; j++){
        // send(sock, payload, strlen(payload), 0);
        // ESP_LOGI(tag, "Message sent:    %s", payload);
        sprintf(payload, "NEW MESSAGE %d", j);
        // ESP_LOGI(tag, "message bofore: %s", payload);
        send_with_header(payload, sock);
        // ESP_LOGI(tag, "Message sent:    %s", payload);

        // DISCONNECT_MSG
        // send(sock, payload, strlen(payload), 0);
        // ESP_LOGI(tag, "Message sent:    %s", payload);
        vTaskDelay(sendSleepTime);
    }
    send_with_header(DISCONNECT_MSG, sock);

    close(sock);
    vTaskDelete(NULL);
}