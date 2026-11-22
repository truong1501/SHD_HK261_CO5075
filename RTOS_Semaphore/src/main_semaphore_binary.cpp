// Week 8 - Class 2: Binary semaphore demo (ESP32-S3, Arduino framework)
// BOOT button (GPIO0) -> binary semaphore -> LED task blinks 3 times
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define BOOT_BUTTON_GPIO  GPIO_NUM_0    // ESP32-S3 BOOT button (active-low)
#define LED_GPIO          GPIO_NUM_48   // LED on GPIO48 (change if your board differs)

SemaphoreHandle_t xBinarySemaphore;     // global handle shared by both tasks

void task_monitor_button(void *pvParameters) {
  while (1) {
    if (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {          // pressed (active-low)
      vTaskDelay(pdMS_TO_TICKS(50));                      // simple debounce
      if (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {
        // Give the semaphore to signal the LED task
        if (xSemaphoreGive(xBinarySemaphore) == pdTRUE)
          Serial.println("Give -> OK");
        else
          Serial.println("Give ignored (semaphore full)");
        // Wait for release: 1 press = 1 Give
        while (gpio_get_level(BOOT_BUTTON_GPIO) == 0)
          vTaskDelay(pdMS_TO_TICKS(10));
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));                        // poll every 10 ms
  }
}

void task_blink_led(void *pvParameters) {
  while (1) {
    // Blocked here (0% CPU) until the button task gives
    if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE) {
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
  pinMode(BOOT_BUTTON_GPIO, INPUT_PULLUP);
  pinMode(LED_GPIO, OUTPUT);

  xBinarySemaphore = xSemaphoreCreateBinary();            // created EMPTY (0)
  if (xBinarySemaphore == NULL) {
    Serial.println("Semaphore creation failed (out of heap)");
    return;
  }

  // Stack size is in BYTES on the ESP32 port
  xTaskCreate(task_monitor_button, "MonitorButton", 4096, NULL, 10, NULL);
  xTaskCreate(task_blink_led,      "BlinkLED",      4096, NULL,  5, NULL);
}

void loop() {
  // empty: the FreeRTOS tasks do all the work
}