#include <Arduino.h>
#include <DHT20.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
// Lưu ý: Cần bao gồm thư viện của cảm biến DHT20 (ví dụ: #include <DHT20.h>) để code hoạt động

#define SENSOR_READ_INTERVAL_MS  5000   // task đọc: 1 mẫu / giây
#define PRINT_INTERVAL_MS        5000   // task in: thức dậy mỗi 5 giây
#define QUEUE_LENGTH              20    // đủ chỗ cho ~5s dữ liệu, dư phòng nếu task in trễ

typedef struct {
  float temperature;
  float humidity;
  unsigned long timestampMs;
} SensorReading_t;

QueueHandle_t xSensorQueue;             // hàng đợi chia sẻ giữa task đọc và task in

void TaskLEDControl(void *pvParameters) {
  pinMode(GPIO_NUM_48, OUTPUT); // Initialize LED pin
  int ledState = 0;
  while(1) {
    if (ledState == 0) {
      digitalWrite(GPIO_NUM_48, HIGH); // Turn ON LED
    } else {
      digitalWrite(GPIO_NUM_48, LOW); // Turn OFF LED
    }
    ledState = 1 - ledState;
    vTaskDelay(2000);
  }
}

// Task đọc: lấy mẫu cảm biến định kỳ và đẩy vào queue
void TaskReadSensor(void *pvParameters) {
  DHT20 dht20;
  Wire.begin(GPIO_NUM_11, GPIO_NUM_12);
  dht20.begin();

  while (1) {
    int status = dht20.read();

    if (status != DHT20_OK) {
      Serial.printf("DHT20 read failed (status = %d), skipping sample\n", status);
    } else {
      SensorReading_t reading;
      reading.temperature = dht20.getTemperature();
      reading.humidity    = dht20.getHumidity();
      reading.timestampMs = millis();

      // timeout = 0: không chờ nếu queue đầy, tránh làm nghẽn task đọc
      if (xQueueSend(xSensorQueue, &reading, 0) != pdTRUE) {
        Serial.println("Queue full, sample dropped");
      }
    }

    vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
  }
}

// Task in: mỗi PRINT_INTERVAL_MS thức dậy 1 lần, in hết toàn bộ các mẫu
// đã tích lũy trong queue kể từ lần in trước (drain queue tới khi rỗng)
void TaskPrintQueue(void *pvParameters) {
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(PRINT_INTERVAL_MS));
    SensorReading_t reading;
    while (xQueueReceive(xSensorQueue, &reading, 0) == pdTRUE) {
      Serial.printf("[t=%lums] Temp: %.2f *C  Humidity: %.2f %%\n",
                    reading.timestampMs, reading.temperature, reading.humidity);
    }
    Serial.println();
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  xSensorQueue = xQueueCreate(QUEUE_LENGTH, sizeof(SensorReading_t));
  if (xSensorQueue == NULL) {
    Serial.println("Queue creation failed (out of heap)");
    return;
  }

  // xTaskCreate(TaskLEDControl, "LED Control", 2048, NULL, 2, NULL);
  xTaskCreate(TaskReadSensor, "Read Sensor", 2048, NULL, 5, NULL);
  xTaskCreate(TaskPrintQueue, "Print Queue", 2048, NULL, 2, NULL);
}

void loop() {
  // Do các tác vụ được quản lý độc lập qua FreeRTOS, hàm loop được để trống
}