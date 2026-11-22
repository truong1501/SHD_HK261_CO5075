#include "global.h"
#include <math.h>

float glob_temperature = NAN;
float glob_humidity = NAN;
portMUX_TYPE sensor_mux = portMUX_INITIALIZER_UNLOCKED;
String ssid = "GoogleBing";
String password = "12345678";
String wifi_ssid;
String wifi_password;
bool isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = nullptr;
