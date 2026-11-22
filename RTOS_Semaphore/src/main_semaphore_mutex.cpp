#include <Arduino.h>
#include <stdarg.h>

#define BOOT_BUTTON_GPIO  GPIO_NUM_0    // nút BOOT, active-low
#define LED_GPIO          GPIO_NUM_48   // LED trên YoloUNO

SemaphoreHandle_t xBinarySemaphore;     // đồng bộ sự kiện: nút nhấn -> task LED
SemaphoreHandle_t xSerialMutex;         // bảo vệ tài nguyên dùng chung: Serial

// In log an toàn: tại một thời điểm chỉ một task được ghi ra Serial
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

// Task 1 (ưu tiên 10): phát hiện nhấn nút -> Give binary semaphore
void task_monitor_button(void *pvParameters) {
  while (true) {
    if (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {
      vTaskDelay(pdMS_TO_TICKS(50));              // chống dội phím
      if (gpio_get_level(BOOT_BUTTON_GPIO) == 0) {
        if (xSemaphoreGive(xBinarySemaphore) == pdTRUE)
          safe_log("GIVE binary semaphore -> OK");
        else
          safe_log("GIVE ignored (semaphore already = 1)");
        while (gpio_get_level(BOOT_BUTTON_GPIO) == 0)  // 1 lần nhấn = 1 lần Give
          vTaskDelay(pdMS_TO_TICKS(10));
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Task 2 (ưu tiên 5): bị block (0% CPU) cho tới khi Take được semaphore
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

// Task 3 (ưu tiên 1): chỉ dùng chung Serial, minh hoạ mutex
void task_heartbeat(void *pvParameters) {
  while (true) {
    safe_log("alive, free heap = %u B", (unsigned)ESP.getFreeHeap());
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BOOT_BUTTON_GPIO, INPUT_PULLUP);
  pinMode(LED_GPIO, OUTPUT);

  xBinarySemaphore = xSemaphoreCreateBinary();   // khởi tạo RỖNG (= 0)
  xSerialMutex     = xSemaphoreCreateMutex();    // khởi tạo SẴN SÀNG (= 1)
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
