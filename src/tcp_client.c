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

    send(socket, payload, strlen(payload), 0);
    ESP_LOGI(tag, "Message sent: \"%s\"", payload);
    // ESP_LOGI(tag, "============== HEADER: %d,  strlen(payload): %d ==============", HEADER, strlen(payload));
}

int closeConnection(int sock){
    ESP_LOGE(tag, "Closing socket and deleting task ...");
    int result = close(sock);
    if (result != 0) {
        ESP_LOGE(tag, "Error closing socket: errno %d", errno);
    }
    connDone = 0;
    retryConnection = true;
    vTaskDelay(pdMS_TO_TICKS(5000));
    startTcp = true;
    ESP_LOGI(tag, "Socket closed");
    vTaskDelete(NULL);
    return 0;
}

void tcp_client_task(void *pvParameters){
    
    // /* init tcp client */
    // /* prepare the audio queue*/
    // ESP_LOGI(tag, "Preparing audio queue ...");
    // audio_queue = xQueueCreate(AUDIO_QUEUE_LENGTH, sizeof(AudioChunk));
    if (audio_queue == NULL) {
        ESP_LOGE("main", "Failed to connect to audio queue!");
    }

    const char *host_ip = DEFAULT_HOST_IP;
    int port = PORT;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(host_ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    int sock = -1;

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
            closeConnection(sock);
            return;
        }
    }
    ESP_LOGI(tag, "Successfully connected");
    ESP_LOGI(tag, "Enabling sending audio data to queue");
    startRecording = true;
    startTcp = false;

    AudioChunk chunk;
    while (1) {
        ESP_LOGI(tag, "main loop of tcp client task");
        if (xQueueReceive(audio_queue, &chunk, portMAX_DELAY) == pdPASS && sock >= 0) {
            int sent = send(sock, chunk.samples, chunk.length, 0);
            if(sent >0){
                ESP_LOGI(tag, "Sent %d bytes", chunk.length);
            }
            else{
                ESP_LOGE(tag, "Error occurred during sending: errno %d", errno);
                startRecording = false; // stop recording when connection is lost
                closeConnection(sock);
                return;
            }
            
        }
        vTaskDelay(pdMS_TO_TICKS(MIC_WAIT));
    }

    close(sock);
    vTaskDelete(NULL);
}