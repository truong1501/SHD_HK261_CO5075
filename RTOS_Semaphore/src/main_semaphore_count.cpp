// Counting semaphore: each BOOT press is counted, so presses made while the LED
// is still blinking are queued (up to SEMAPHORE_MAX_COUNT) instead of being lost.
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define BOOT_BUTTON_GPIO  GPIO_NUM_0    // ESP32-S3 BOOT button (active-low)
#define LED_GPIO          GPIO_NUM_48
#define SEMAPHORE_MAX_COUNT 5

SemaphoreHandle_t xCountingSemaphore;

void task_monitor_button(void *pvParameters) {
  while (1) {
    if (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {          // pressed (active-low)
      vTaskDelay(pdMS_TO_TICKS(50));                      // simple debounce
      if (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {
        if (xSemaphoreGive(xCountingSemaphore) == pdTRUE)
          Serial.printf("Give -> OK (count = %u)\n", uxSemaphoreGetCount(xCountingSemaphore));
        else
          Serial.println("Give ignored (semaphore at max count)");
        // One press = one give: wait for release
        while (gpio_get_level(BOOT_BUTTON_GPIO) == 0)
          vTaskDelay(pdMS_TO_TICKS(10));
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));                        // poll every 10 ms
  }
}

void task_blink_led(void *pvParameters) {
  while (1) {
    // Blocks without consuming CPU until a press is pending
    if (xSemaphoreTake(xCountingSemaphore, portMAX_DELAY) == pdTRUE) {
      for (int i = 0; i < 3; i++) {                       // blink 3 times
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
      }
    }
  }
}

void setup(void) {
  Serial.begin(115200);
  // Wait up to 3 s for the USB CDC host; press RST with the monitor open to replay the boot log
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) delay(10);
  Serial.println("Counting semaphore demo (max 5): press BOOT several times quickly");
  pinMode(BOOT_BUTTON_GPIO, INPUT_PULLUP);
  pinMode(LED_GPIO, OUTPUT);

  // Starts empty; accumulates up to SEMAPHORE_MAX_COUNT pending gives
  xCountingSemaphore = xSemaphoreCreateCounting(SEMAPHORE_MAX_COUNT, 0);
  if (xCountingSemaphore == NULL) {
    Serial.println("Semaphore creation failed (out of heap)");
    return;
  }

  // Stack depth is in bytes on the ESP32 port
  xTaskCreate(task_monitor_button, "MonitorButton", 4096, NULL, 10, NULL);
  xTaskCreate(task_blink_led,      "BlinkLED",      4096, NULL,  5, NULL);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}