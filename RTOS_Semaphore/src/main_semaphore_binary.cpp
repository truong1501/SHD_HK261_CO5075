// Binary semaphore: BOOT button task signals the LED task, which blinks 3 times per event.
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define BOOT_BUTTON_GPIO  GPIO_NUM_0    // ESP32-S3 BOOT button (active-low)
#define LED_GPIO          GPIO_NUM_48

SemaphoreHandle_t xBinarySemaphore;

void task_monitor_button(void *pvParameters) {
  while (1) {
    if (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {          // pressed (active-low)
      vTaskDelay(pdMS_TO_TICKS(50));                      // simple debounce
      if (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {
        if (xSemaphoreGive(xBinarySemaphore) == pdTRUE)
          Serial.println("Give -> OK");
        else
          Serial.println("Give ignored (semaphore full)");
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
    // Blocks without consuming CPU until the button task gives
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
  // Wait up to 3 s for the USB CDC host; press RST with the monitor open to replay the boot log
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) delay(10);
  Serial.println("Binary semaphore demo: press BOOT -> LED blinks 3 times");
  pinMode(BOOT_BUTTON_GPIO, INPUT_PULLUP);
  pinMode(LED_GPIO, OUTPUT);

  xBinarySemaphore = xSemaphoreCreateBinary();            // starts empty
  if (xBinarySemaphore == NULL) {
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