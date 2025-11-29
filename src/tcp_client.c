#include "wifi.h"
#include "tcp_client.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "mic.h"
#include "main.h"
#include "esp_timer.h"
#include <math.h>


static const char* tag = "tcp";
static const int maxConn = 50;

static bool retryConnection = true;
static int connDone = 0;
static int sock = -1;
static bool doManSync = false;
static int doSyncCounter = 0;
static int seq = 0;


static int64_t offsetUs = 0;

typedef struct __attribute__((packed)) {
    uint8_t type;      // 'A'
    int64_t ts_us;     
    int64_t seq;       
} audio_hdr_t; // send audio data header

typedef struct __attribute__((packed)) {
    uint8_t type;   // 'M'
    int64_t ts_us;
} sync_hdr_t; // receive manual sync header

typedef struct __attribute__((packed)){
    uint8_t type;   // 'Q'
    int64_t t1_us;  
    int64_t seq;    
} sync_query_t; // send sync query

typedef struct __attribute__((packed)){
    uint8_t type;   // 'R' (reply)
    int64_t t1_us;
    int64_t t2_us;
    int64_t t3_us;
} sync_reply_t; // receive sync reply

static int64_t baseRttUs = INT64_MAX;


void send_sync_query() {
    sync_query_t q = { 'Q', esp_timer_get_time() , seq++};
    send(sock, &q, sizeof(q), 0);
}

static int recv_all_exact(int s, void *buf, size_t len, int flags) {
    uint8_t *p = (uint8_t*)buf;
    size_t got = 0;
    while (got < len) {
        int n = recv(s, p + got, len - got, flags);
        if (n == 0)  return 0;                  
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return -2; 
            return -1;                       
        }
        got += n;
    }
    return (int)got;
}


static void handle_incoming_messages(void) {
    uint8_t type;
    int n = recv(sock, &type, 1, MSG_DONTWAIT);
    if (n <= 0) return;
    


    switch (type) {
        case 'R': {  
            sync_reply_t r;
            r.type = 'R';
            int rc = recv_all_exact(sock, ((uint8_t*)&r) + 1, sizeof(r) - 1, 0);
            if (rc == sizeof(r) - 1) {
                int64_t t4  = esp_timer_get_time();
                int64_t rtt = (t4 - r.t1_us) - (r.t3_us - r.t2_us);

               
                if (rtt > 0 && rtt < baseRttUs) baseRttUs = rtt;  

                bool syncCondition = false;
                if (rtt > 0 && (rtt - baseRttUs) < 2000)   
                    syncCondition = true;
                
                double quality = fmax(0.0, fmin(1.0, (double)baseRttUs / (double)rtt));
                double alpha = 0.9 * (1.0 - quality);  
                double beta  = 1.0 - alpha;            

                
                if (doManSync || doSyncCounter < 20) {
                    syncCondition = (rtt >= 0);   
                    alpha = 0.2;                  
                    beta  = 0.8;                  
                    doManSync = false;
                }
                if (syncCondition) { 
                    int64_t newOffset = ((r.t2_us - r.t1_us) + (r.t3_us - t4)) / 2;
                    offsetUs = (int64_t)(alpha * (double)offsetUs + beta * (double)newOffset);
                    synchOffsetUs = offsetUs;
                    // ESP_LOGI(tag,
                    //     "SYNC 'R': rtt=%lld us, offset=%lld us, t1=%lld us, t2=%lld us, t3=%lld us, t4=%lld us",
                    //     (long long)rtt, (long long)offsetUs,
                    //     (long long)r.t1_us, (long long)r.t2_us, (long long)r.t3_us, (long long)t4);

                    doSyncCounter++;
                }
            }
            break;
        }

        case 'M': { 
            sync_hdr_t m;
            m.type = 'M';
            int rc = recv_all_exact(sock, ((uint8_t*)&m) + 1, sizeof(m) - 1, 0);
            if (rc == sizeof(m) - 1) {
                int64_t now = esp_timer_get_time();
                int64_t diff = m.ts_us - now;
                synchOffsetUs = diff;
                ESP_LOGI(tag, "MANUAL SYNC 'M': server=%lld, local=%lld, diff=%lld us",
                         (long long)m.ts_us, (long long)now, (long long)diff);
                doManSync = true;
                doSyncCounter = 0;
            }
            break;
        }

        default:
            
            ESP_LOGW(tag, "Unknown TCP msg type '%c' (0x%02x)", type, type);
            break;
    }
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
static int send_all(int s, const void *buf, size_t len) {
    const uint8_t *p = buf;
    size_t sent = 0;
    while (sent < len) {
        int n = send(s, p + sent, len - sent, 0);
        if (n <= 0) return -1;
        sent += n;
    }
    return sent;
}


int send_audio_chunk(AudioChunk *chunk){
    if(xQueueReceive(audio_queue, chunk, portMAX_DELAY) != pdPASS){
        #if LOG_AUDIO
            ESP_LOGE(tag, "Failed to receive audio chunk from queue");
        #endif
        return 0;
    }
    audio_hdr_t h = { 'A', chunk->timestamp, chunk->messageNum };

    if (send_all(sock, &h, sizeof(h)) < 0 ||
        send_all(sock, chunk->samples, chunk->length) < 0) {
        ESP_LOGE(tag, "Send failed: errno %d", errno);
        startRecording = false;
        closeConnection(sock);
        return -1;
    }


    return (int)chunk->length;

    }




void run_tcp_client_task(void *pvParameters){
    init_tcp();

    AudioChunk chunk;


    while (1) {

        int res = send_audio_chunk(&chunk);
        if(res < 0){ 
            closeConnection(sock);  
        }

        
        static int64_t last_sync_q = 0;
        int64_t now = esp_timer_get_time();
        if (now - last_sync_q >= 500000) { 
            send_sync_query();            
            last_sync_q = now;
        }
        handle_incoming_messages();     

        vTaskDelay(pdMS_TO_TICKS(MIC_WAIT));
    }
    close(sock);
    vTaskDelete(NULL);
}



void try2connect_tcp_task(){
        while(1){ 
        if(startTcp){
            ESP_LOGI("main", "Restarting TCP client task ...");
            vTaskDelay(pdMS_TO_TICKS(5000));

            xTaskCreatePinnedToCore(run_tcp_client_task, "tcp_client", TCP_STACK_SIZE, NULL, PRI_HIGH, NULL, CORE_APP);
            startTcp = false;
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
