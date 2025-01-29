#include "driver/gpio.h"
#include "http_lib.h"
#include "wifi_lib.h"
#include "display_lib.h"


#define LED_GPIO 25
#define TEMPO_ACESSO 0.1 // Tempo de acesso liberado em minutos

volatile uint8_t access_control = 0;
volatile uint8_t db_carregado = 0, promiscuous_active=0;
volatile uint8_t state=0;

volatile uint16_t mac_counter = 0;
volatile char *macs[12];
volatile char *nomes[16];
volatile uint8_t idx = 0;

void machine(void *pvParameters){

    TaskHandle_t sniffer_handler = NULL;
    while (1){
        switch (state){
        case 0:
            wifi_init_sta(NULL);
            if (db_carregado)
                state = 3;
            else   
                state = 1;
            break;
        
        case 1:
            http_get_task();                   
            db_carregado = 1;
            state = 2;
            break;

        case 2:
            if (!promiscuous_active){
                display_clear();
                vTaskDelay(100/portTICK_PERIOD_MS);
                display_text((void *)"\n\n\nAproxime o dis-\n\npositivo       \n"); 

                xTaskCreate(&sniffer_task, "sniffer_task", 4096, NULL, 5, &sniffer_handler);
                promiscuous_active = 1;
            }
            else if (access_control){
                if (sniffer_handler != NULL){
                    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(false));
                    ESP_ERROR_CHECK(esp_wifi_stop());
                    vTaskDelete(sniffer_handler);
            }
                state = 0;
                promiscuous_active = 0;

                display_clear();
                vTaskDelay(100/portTICK_PERIOD_MS);
                char txt[40] = "\n\n\nAcesso Liberado\n\n";
                strncat(txt, nomes[idx],16);
                display_text((void *)txt);                
            }
            else{
                vTaskDelay(10/portTICK_PERIOD_MS);
            }
            break;
        
        case 3:
            http_post_task();

            //Contador de tempo da sessão
            char seg[11] = "\n00:00";
            for (uint16_t i=60*TEMPO_ACESSO; i>0;i--){
                sprintf(seg, "\n%02d:%02d", i/60, i%60);
                display_text((void *)seg);
                vTaskDelay(1000/portTICK_PERIOD_MS);
            }  

            display_text((void *)"\n               ");                          
            access_control = 0;
            state = 2;
            break;
        }
    }
}


void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    /* Configura a porta GPIO como push/pull output */
    gpio_pad_select_gpio(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);


    i2c_master_init();
	ssd1306_init();
    display_clear();
    vTaskDelay(100/portTICK_PERIOD_MS);
    display_text((void *)"\n\n\nCarregando");

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler_promiscuous, NULL) );
    esp_netif_create_default_wifi_sta();

    xTaskCreate(&machine, "state_machine", 8192, NULL, 5, NULL);
    while(1){
        gpio_set_level(LED_GPIO, access_control);
        vTaskDelay(10/portTICK_PERIOD_MS);
    }

}