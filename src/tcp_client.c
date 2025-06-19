#include "wifi.h"
#include "tcp_client.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "mic.h"


// static const uint32_t sendSleepTime = 1000/portTICK_PERIOD_MS;
static const char* tag = "tcp";
static const int maxConn = 50;

static bool retryConnection = true;
static int connDone = 0;

void send_with_header(char *payload, int socket){
    // /* send the message length */
    // uint payload_lenthg = strlen(payload);
    // char payload_lenthg_msg[HEADER] = {0};
    // snprintf(payload_lenthg_msg, HEADER, "%d", payload_lenthg);
    // send(socket, payload_lenthg_msg, HEADER, 0); 
    // ESP_LOGI(tag, "Message lenght sent: \"%s\"", payload_lenthg_msg);
    
    /* send the actual message */
    send(socket, payload, strlen(payload), 0);
    ESP_LOGI(tag, "Message sent: \"%s\"", payload);
    // ESP_LOGI(tag, "============== HEADER: %d,  strlen(payload): %d ==============", HEADER, strlen(payload));
}


void tcp_client_task(void *pvParameters){
    /* init tcp client */
    /* prepare the audio queue*/
    ESP_LOGI(tag, "Preparing audio queue ...");
    audio_queue = xQueueCreate(AUDIO_QUEUE_LENGTH, sizeof(AudioChunk));
    if (audio_queue == NULL) {
        ESP_LOGE("main", "Failed to create audio queue!");
    }

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

    // /* run the tcp client */
    // char payload[PAYLOAD_LEN];
    // for(int j=0; j<3; j++){

    //     sprintf(payload, "Hello from esp32 %d", j);

    //     send_with_header(payload, sock);

    //     vTaskDelay(sendSleepTime);
    // }
    // send_with_header(DISCONNECT_MSG, sock);
    AudioChunk chunk;
    while (1) {
        if (xQueueReceive(audio_queue, &chunk, portMAX_DELAY) == pdPASS) {
            if (sock > 0) {
                send(sock, chunk.samples, chunk.length, 0);
                ESP_LOGI(tag, "Sent %d bytes", chunk.length);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(MIC_WAIT));
    }

    close(sock);
    vTaskDelete(NULL);
}