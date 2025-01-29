#include "http_lib.h"

extern volatile uint16_t mac_counter;
extern volatile char * macs[12];
extern volatile char *nomes[16];
extern uint8_t idx;


static const char *TAG1 = "status";

static const char *REQUEST = "GET " WEB_PATH " HTTP/1.1\r\n"
    "Host: "WEB_SERVER":"WEB_PORT"\r\n"
    //"Host: "WEB_SERVER"\r\n"
    "Content-Type: application/json\r\n"
    //"User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

void http_get_task()
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];

    while(1) {
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG1, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.
           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG1, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG1, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG1, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG1, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG1, "... connected");
        freeaddrinfo(res);

        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
            ESP_LOGE(TAG1, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG1, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG1, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG1, "... set socket receiving timeout success");

        /* Ler resposta HTTP e filtrar apenas os endereços MAC*/

        uint8_t head_mac_counter = 0, str_counter = 0, ready_mac = 0;
        uint8_t ready_nome = 0, head_nome_counter = 0;
        char head_mac[6] = "mac\":\"";
        char head_nome[7] = "nome\":\"";

        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            for(int i = 0; i < r; i++) {
                char c = recv_buf[i];
                // putchar(c);
                if(ready_mac){
                    macs[mac_counter-1][str_counter] = c;
                    if (str_counter<11){
                        str_counter++;
                    }
                    else{
                        ready_mac=0;
                        str_counter=0;
                    }
                }
                else if(ready_nome){
                    nomes[mac_counter-1][str_counter] = c;
                    if (c != '"' && str_counter<15){
                        str_counter++;
                    }
                    else{
                        for (uint8_t j = str_counter; j < 16; j++)
                        {
                            nomes[mac_counter-1][j] = ' ';
                        }
                        
                        ready_nome=0;
                        str_counter=0;
                    }
                }
                
                else{
                    if (c == '{'){
                        macs[mac_counter] = (char*)calloc(12,sizeof(char));
                        nomes[mac_counter] = (char*)calloc(16,sizeof(char));
                        mac_counter++;
                    }
                    else if(c == head_nome[head_nome_counter]){
                        if (head_nome_counter<6){
                            head_nome_counter++;
                        }
                        else{
                            head_nome_counter = 0;
                            ready_nome = 1;
                        }
                    }
                    else if(c == head_mac[head_mac_counter]){
                        if (head_mac_counter<5){
                            head_mac_counter++;
                        }
                        else{
                            head_mac_counter = 0;
                            ready_mac = 1;
                        }
                    }
                    else{
                        head_mac_counter = 0;
                        head_nome_counter = 0;
                    }
                }
            }
        } while(r > 0);

        ESP_LOGI(TAG1, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
        close(s);
        break;
    }
}

void http_post_task(){
    char mac[13] = "000000000000";
    for (uint8_t i = 0; i < 12; i++)
    {
        mac[i] = macs[idx][i];
    }
    
    char REQUEST_POST[128] = "POST ";
    strcat(REQUEST_POST, WEB_PATH_POST);
    strcat(REQUEST_POST, mac);
    strcat(REQUEST_POST, " HTTP/1.1\r\n");
    strcat(REQUEST_POST, "Host: ");
    strcat(REQUEST_POST, WEB_SERVER);
    strcat(REQUEST_POST, ":");
    strcat(REQUEST_POST, WEB_PORT);
    strcat(REQUEST_POST, "\r\nContent-Type: application/json\r\n\r\n");

    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s;
    char recv_buf[64];

    while(1) {
    int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);


        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG1, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.
           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG1, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG1, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG1, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG1, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG1, "... connected");
        freeaddrinfo(res);

        if (write(s, REQUEST_POST, strlen(REQUEST_POST)) < 0) {
            ESP_LOGE(TAG1, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG1, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG1, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG1, "... set socket receiving timeout success");

        close(s);
        break;
    }
}