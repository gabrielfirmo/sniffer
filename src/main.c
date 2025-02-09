#include "driver/gpio.h"
#include "wifi_lib.h"


#define LED_GPIO 25


void app_main(void)
{
    TaskHandle_t sniffer_handler = NULL;
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


    // i2c_master_init();
	// ssd1306_init();
    // display_clear();
    // vTaskDelay(100/portTICK_PERIOD_MS);
    // display_text((void *)"\n\n\nCarregando");

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler_promiscuous, NULL) );
    esp_netif_create_default_wifi_sta();
    
    xTaskCreate(&sniffer_task, "sniffer_task", 4096, NULL, 5, &sniffer_handler);
    

}