#include <Arduino.h>

#define LED_PIN        48
#define BLINK_HALF_MS  1000   // on time = off time

void led_blinky(void *pvParameters) {
  pinMode(LED_PIN, OUTPUT);
  bool ledOn = false;
  TickType_t lastWake = xTaskGetTickCount();

  while (true) {
    ledOn = !ledOn;
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    Serial.printf("[%8lu ms] LED %s\n", millis(), ledOn ? "ON" : "OFF");

    // Fixed period: wake-up time does not drift with the work done in the loop
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(BLINK_HALF_MS));
  }
}

void setup() {
  Serial.begin(115200);
  xTaskCreate(led_blinky, "LED Blinky", 2048, NULL, 2, NULL);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
