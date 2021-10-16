#ifndef HTTP_LIB
#define HTTP_LIB

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

/* Informações do servidor WEB */
#define WEB_SERVER "sniffer-esp.herokuapp.com"
#define WEB_PORT "80"
#define WEB_PATH "/macs"
#define WEB_PATH_POST "/logs/"

void http_get_task();
void http_post_task();

#endif