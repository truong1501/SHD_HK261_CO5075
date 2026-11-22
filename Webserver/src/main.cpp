#include <Arduino.h>
#include "global.h"
#include "mainserver.h"
#include "temp_humi_monitor.h"

void setup() {
    Serial.begin(115200);

    // Tao semaphore truoc khi server gui tin hieu ket noi Wi-Fi.
    xBinarySemaphoreInternet = xSemaphoreCreateBinary();
    if (xBinarySemaphoreInternet == nullptr) {
        Serial.println("Cannot create Internet semaphore");
        return;
    }
    // Task doc nhiet do va do am, cap nhat cac bien dung chung.
    if (xTaskCreate(temp_humi_monitor, "DHT20", 4096,
                    nullptr, 2, nullptr) != pdPASS) {
        Serial.println("Cannot create DHT20 task");
    }
    // Task quan ly Wi-Fi va xu ly cac yeu cau HTTP.
    if (xTaskCreate(main_server_task, "Web Server", 8192,
                    nullptr, 2, nullptr) != pdPASS) {
        Serial.println("Cannot create Web Server task");
    }
}

void loop() {
    // Hai task tren tu chay; loop chi nhuong thoi gian CPU.
    delay(1000);
}
