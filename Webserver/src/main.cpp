#include <Arduino.h>
#include "global.h"
#include "mainserver.h"
#include "temp_humi_monitor.h"
#include "coreiot_task.h"

void setup() {
    Serial.begin(115200);
    // Wait up to 3 s for the USB CDC host so the IP address log is not lost.
    // Press RST with the Serial Monitor open to replay the boot log.
    uint32_t t0 = millis();
    while (!Serial && millis() - t0 < 3000) {
        delay(10);
    }

    // Given by the web server task once the router connection is up
    xBinarySemaphoreInternet = xSemaphoreCreateBinary();
    if (xBinarySemaphoreInternet == nullptr) {
        Serial.println("Cannot create Internet semaphore");
        return;
    }
    if (xTaskCreate(temp_humi_monitor, "DHT20", 4096,
                    nullptr, 2, nullptr) != pdPASS) {
        Serial.println("Cannot create DHT20 task");
    }
    if (xTaskCreate(main_server_task, "Web Server", 8192,
                    nullptr, 2, nullptr) != pdPASS) {
        Serial.println("Cannot create Web Server task");
    }
    if (xTaskCreate(coreiot_task, "CoreIOT", 6144,
                    nullptr, 1, nullptr) != pdPASS) {
        Serial.println("Cannot create CoreIOT task");
    }
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
