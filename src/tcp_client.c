#include "wifi.h"
#include "tcp_client.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "mic.h"
#include "main.h"
#include "esp_timer.h"

static const char* tag = "tcp";
static const int maxConn = 50;

static bool retryConnection = true;
static int connDone = 0;
static int sock = -1;

static char message_buf[128];
static int message_len = 0;



// sock, &chunk->timestamp, sizeof(chunk->timestamp

int send_with_header(int socket, const void *dataptr, size_t data_len, int flags, char headertyp, char* headerRest){

    uint8_t header[TCP_HEADER_LEN] = {0};
    header[0] = headertyp;
    memcpy(header + 1, headerRest, TCP_HEADER_LEN - 1);

    uint8_t *msg = malloc(TCP_HEADER_LEN + data_len);
    if (!msg) return -1;
    memcpy(msg, header, TCP_HEADER_LEN);
    memcpy(msg + TCP_HEADER_LEN, dataptr, data_len);

    #if LOG_DATA
        printf("\n");
        ESP_LOGI(tag, "message");
        for(int i=0; i<TCP_HEADER_LEN; i++){
            printf("%02X ", msg[i]);
        }
        printf("| ");
        for(int i=TCP_HEADER_LEN; i<TCP_HEADER_LEN + data_len; i++){
            printf("%02X ", msg[i]);
        }
        printf("\n");
    #endif

    int sent = send(socket, msg, TCP_HEADER_LEN + data_len, flags);
    free(msg);
    return sent;
}


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
    if(xQueueReceive(audio_queue, chunk, portMAX_DELAY) == pdPASS) {
        // int sentAudioRes = send(sock, chunk->samples, chunk->length, 0);
        int sentAudioRes = send_with_header(sock, chunk->samples, chunk->length, 0, 'A', (char*)&(chunk->timestamp) );
        // int sentAudioRes = send(sock, chunk->samples, chunk->length, 0);
        // int sentTs = send_with_header(sock, &chunk->timestamp, sizeof(chunk->timestamp), 0);
        if(sentAudioRes >0){ // success
            #if LOG_AUDIO
                ESP_LOGI(tag, "timestamp: %lld, Sent %d bytes to the server", chunk->timestamp, chunk->length);
            #endif
            return sentAudioRes;
        }
        else{
            ESP_LOGE(tag, "Error occurred during sending audio: errno %d", errno);
            startRecording = false; // stop recording when connection is lost
            return -1;
        }   
    }
    else{
        #if LOG_AUDIO
            ESP_LOGE(tag, "Failed to receive audio chunk from queue");
        #endif
        return 0;
    }

    // /* timestamps */
    // int64_t t0;
    // if(sentAudioRes){
    //     if(xQueueReceive(timestamp_queue, &t0, portMAX_DELAY) == pdPASS) {
    //         int sent = send(sock, &t0, sizeof(t0), 0);
    //         #if LOG_AUDIO
    //             ESP_LOGI(tag, "Sent timestamp %lld to the server\n", t0);
    //         #endif
    //     }
    // }
    // else{
    //     #if LOG_AUDIO 
    //         ESP_LOGE(tag, "Failed to receive timestamp from queue");
    //     #endif
    // }
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
        int res = send_audio_chunk(&chunk);
        if(res < 0){ // failure
            closeConnection(sock);  // close connection
        }
        else if (res > 0){ // success
            // int64_t us_since_start = esp_timer_get_time(); // microseconds since boot
            // ESP_LOGI("main", "Microseconds since start: %lld", us_since_start);
        }
        

        /* receive message from server */
        if(recv_message(recv_buf, message_out)){
            ESP_LOGI(tag, "Received message: \"%s\"", message_out);

            if (strcmp(message_out, DISCONNECT_MSG) == 0) {
                ESP_LOGW(tag, "Received disconnect message from server");
                closeConnection(sock);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(MIC_WAIT));
    }
    close(sock);
    vTaskDelete(NULL);
}