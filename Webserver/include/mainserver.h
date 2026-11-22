#ifndef ___MAIN_SERVER__
#define ___MAIN_SERVER__
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "global.h"

#define LED1_PIN 48      // D13, on-board single LED (same pin as LedBlinky)
#define LED2_PIN 45      // on-board WS2812 RGB LED
#define BOOT_PIN 0
//extern WebServer server;

//extern bool isAPMode;




void startAP();
void setupServer();
void connectToWiFi();

void main_server_task(void *pvParameters);

#endif