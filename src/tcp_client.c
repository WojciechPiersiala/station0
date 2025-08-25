#include "wifi.h"
#include "tcp_client.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "mic.h"
#include "main.h"

static const char* tag = "tcp";
static const int maxConn = 50;

static bool retryConnection = true;
static int connDone = 0;
static int sock = -1;

static char message_buf[128];
static int message_len = 0;

// void send_with_header(char *payload, int socket){

//     send(socket, payload, strlen(payload), 0);
//     ESP_LOGI(tag, "Message sent: \"%s\"", payload);
// }


int closeConnection(int sock){
    ESP_LOGE(tag, "Closing socket and deleting task ...");
    int result = close(sock);
    if (result != 0) {
        ESP_LOGE(tag, "Error closing socket: errno %d", errno);
    }
    connDone = 0;
    retryConnection = true;
    vTaskDelay(pdMS_TO_TICKS(1000));
    startTcp = true;
    ESP_LOGI(tag, "Socket closed");
    vTaskDelete(NULL);
    return 0;
}


void init_tcp(){
    /* init */
    if (audio_queue == NULL) {
        ESP_LOGE("main", "Failed to connect to audio queue!");
        vTaskDelete(NULL);
    }

    const char *host_ip = DEFAULT_HOST_IP;
    int port = PORT;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(host_ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);


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
}


int send_audio_chunk(AudioChunk *chunk){
    if (xQueueReceive(audio_queue, chunk, portMAX_DELAY) == pdPASS && sock >= 0) {
        int sent = send(sock, chunk->samples, chunk->length, 0);
        if(sent >0){
            if(LOG_AUDIO){
                ESP_LOGI(tag, "Sent %d bytes to the server", chunk->length);
            }
            return sent;
        }
        else{
            ESP_LOGE(tag, "Error occurred during sending audio: errno %d", errno);
            startRecording = false; // stop recording when connection is lost
            return -1;
        }   
    }
    else{
        if(LOG_AUDIO){
            ESP_LOGE(tag, "Failed to receive audio chunk from queue");
        }
        return 0;
    }
}

int recv_message(char *recv_buf, char* message_out){
    int len = recv(sock, recv_buf, sizeof(recv_buf)-1, MSG_DONTWAIT);
    if (len > 0) {
        // ESP_LOGI(tag, "Received message: %s", recv_buf);
        recv_buf[len] = 0;
        for(int i=0; i<len; i++){
            message_buf[message_len++] = recv_buf[i];
            if(recv_buf[i] == '\n'){
                message_buf[message_len] = 0;

                // strcpy(message_out, message_buf);
                strncpy(message_out, message_buf, message_len - 1);
                message_out[message_len - 1] = 0;

                message_len = 0;
                return strlen(message_out); // return the length of the received message
            }
        }

    }
    return 0; // no message received
}


void tcp_client_task(void *pvParameters){
    init_tcp();

    AudioChunk chunk;
    char recv_buf[128];
    char message_out[128];

    while (1) {
        /* send audio chunk*/
        if(send_audio_chunk(&chunk) < 0){
            closeConnection(sock);  
        }
        vTaskDelay(pdMS_TO_TICKS(MIC_WAIT));

        /* receive message from server */
        if(recv_message(recv_buf, message_out)){
            ESP_LOGI(tag, "Received message: \"%s\"", message_out);

            if (strcmp(message_out, DISCONNECT_MSG) == 0) {
                ESP_LOGW(tag, "Received disconnect message from server");
                closeConnection(sock);
            }
        }

    }
    close(sock);
    vTaskDelete(NULL);
}