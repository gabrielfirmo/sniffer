#ifndef WIFI_LIB
#define WIFI_LIB

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi_types.h"

//Dados de configuração do Access Point (AP) wifi
#define AP_SSID      "TP-Link_396C"
#define AP_PASS      "16711020"
#define MAXIMUM_RETRY  3

/* Eventos possíveis
 * - conexão bem sucedida com o AP
 * - falha na conexão após máximo de tentativas */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


#define	WIFI_CHANNEL_MAX		(13)
#define	WIFI_CHANNEL_SWITCH_INTERVAL	(50)


void wifi_init_sta(void *params);

//Estrutura do pacote recebido pelo sniffer
typedef struct {
	unsigned frame_ctrl:16;
	unsigned duration_id:16;
	uint8_t addr1[6]; /* receiver address */
	uint8_t addr2[6]; /* sender address */
	uint8_t addr3[6]; /* filtering address */
	unsigned sequence_ctrl:16;
	uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
	wifi_ieee80211_mac_hdr_t hdr;
	uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

uint8_t compare_mac(wifi_ieee80211_mac_hdr_t *hdr, char mac[]);

esp_err_t event_handler_promiscuous(void *ctx, system_event_t *event);

void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type);

void promiscuous_init(void);

void sniffer_task(void *pvParameters);

#endif