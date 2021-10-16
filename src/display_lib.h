#ifndef DISPLAY_LIB
#define DISPLAY_LIB

#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
// #include "freertos/task.h"

#include "sdkconfig.h"

#include "ssd1366.h"

#define SDA_PIN GPIO_NUM_4
#define SCL_PIN GPIO_NUM_15
#define RST_PIN GPIO_NUM_16

#define tag "SSD1306"

void i2c_master_init();

void ssd1306_init();

void display_clear();

void display_text(const void *arg_text);

#endif