#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

extern float glob_temperature;
extern float glob_humidity;
extern portMUX_TYPE sensor_mux;
extern String ssid;
extern String password;
extern String wifi_ssid;
extern String wifi_password;
extern bool isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;
