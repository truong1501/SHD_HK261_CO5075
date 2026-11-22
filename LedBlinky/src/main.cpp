// platformio.ini: board = yolo_uno, framework = arduino, monitor_speed = 115200
#include <Arduino.h>

#define LED_PIN        48      // LED đơn trên YoloUNO (GPIO48)
#define BLINK_HALF_MS  1000    // sáng 1000 ms, tắt 1000 ms -> chu kỳ 2000 ms

// Task FreeRTOS điều khiển LED, không dùng delay() trong loop()
void led_blinky(void *pvParameters) {
  pinMode(LED_PIN, OUTPUT);
  bool ledOn = false;
  TickType_t lastWake = xTaskGetTickCount();   // mốc thời gian để giữ chu kỳ cố định

  while (true) {
    ledOn = !ledOn;                                 // đảo trạng thái LED
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    Serial.printf("[%8lu ms] LED %s\n", millis(), ledOn ? "ON" : "OFF");

    // vTaskDelayUntil: đánh thức đúng mỗi 1000 ms tính từ lastWake,
    // không bị trôi chu kỳ do thời gian thực thi Serial.printf
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(BLINK_HALF_MS));
  }
}

void setup() {
  Serial.begin(115200);
  // Stack 2048 byte (trên ESP32 đơn vị là byte), độ ưu tiên 2
  xTaskCreate(led_blinky, "LED Blinky", 2048, NULL, 2, NULL);
}

void loop() {
  vTaskDelay(portMAX_DELAY);   // loopTask không làm gì, nhường CPU cho các task khác
}
