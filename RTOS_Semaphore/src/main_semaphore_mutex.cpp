#include <Arduino.h>
#include <stdarg.h>

#define BOOT_BUTTON_GPIO  GPIO_NUM_0    // active-low
#define LED_GPIO          GPIO_NUM_48

SemaphoreHandle_t xBinarySemaphore;     // event signal: button task -> LED task
SemaphoreHandle_t xSerialMutex;         // guards the shared Serial port

// Thread-safe log line, prefixed with uptime and the calling task name
void safe_log(const char *fmt, ...) {
  char msg[96];
  va_list args;
  va_start(args, fmt);
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);
  if (xSemaphoreTake(xSerialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    Serial.printf("[%7lu ms][%-13s] %s\n", millis(), pcTaskGetName(NULL), msg);
    xSemaphoreGive(xSerialMutex);
  }
}

// Priority 10: debounce the BOOT button and give the binary semaphore
void task_monitor_button(void *pvParameters) {
  while (true) {
    if (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {
      vTaskDelay(pdMS_TO_TICKS(50));              // debounce
      if (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {
        if (xSemaphoreGive(xBinarySemaphore) == pdTRUE)
          safe_log("GIVE binary semaphore -> OK");
        else
          safe_log("GIVE ignored (semaphore already = 1)");
        while (gpio_get_level(BOOT_BUTTON_GPIO) == 0)  // one press = one give
          vTaskDelay(pdMS_TO_TICKS(10));
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Priority 5: blocked until the semaphore is given, then blinks the LED 3 times
void task_blink_led(void *pvParameters) {
  while (true) {
    safe_log("Waiting to TAKE binary semaphore...");
    if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE) {
      safe_log("TAKE OK -> blink LED 3 times");
      for (int i = 0; i < 3; i++) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
      }
      safe_log("Blink done");
    }
  }
}

// Priority 1: periodic heartbeat sharing the Serial port through the mutex
void task_heartbeat(void *pvParameters) {
  while (true) {
    safe_log("alive, free heap = %u B", (unsigned)ESP.getFreeHeap());
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void setup() {
  Serial.begin(115200);
  // Wait up to 3 s for the USB CDC host; press RST with the monitor open to replay the boot log
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) delay(10);
  Serial.println("Binary semaphore + mutex demo: press BOOT, watch TAKE/GIVE log");
  pinMode(BOOT_BUTTON_GPIO, INPUT_PULLUP);
  pinMode(LED_GPIO, OUTPUT);

  xBinarySemaphore = xSemaphoreCreateBinary();   // starts empty
  xSerialMutex     = xSemaphoreCreateMutex();    // starts available
  if (xBinarySemaphore == NULL || xSerialMutex == NULL) {
    Serial.println("Semaphore creation failed (out of heap)");
    return;
  }

  xTaskCreate(task_monitor_button, "MonitorButton", 4096, NULL, 10, NULL);
  xTaskCreate(task_blink_led,      "BlinkLED",      4096, NULL,  5, NULL);
  xTaskCreate(task_heartbeat,      "Heartbeat",     4096, NULL,  1, NULL);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
