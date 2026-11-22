#include <Arduino.h>
#include <DHT20.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define SENSOR_READ_INTERVAL_MS  5000
#define PRINT_INTERVAL_MS        5000
#define QUEUE_LENGTH             20     // headroom if the printer task falls behind

typedef struct {
  float temperature;
  float humidity;
  unsigned long timestampMs;
} SensorReading_t;

QueueHandle_t xSensorQueue;

// Producer: sample the sensor periodically and push readings to the queue
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

      // Non-blocking send: drop the sample rather than stall the producer
      if (xQueueSend(xSensorQueue, &reading, 0) != pdTRUE) {
        Serial.println("Queue full, sample dropped");
      }
    }

    vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
  }
}

// Consumer: wake every PRINT_INTERVAL_MS and drain all queued readings
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
  Serial.begin(115200);

  xSensorQueue = xQueueCreate(QUEUE_LENGTH, sizeof(SensorReading_t));
  if (xSensorQueue == NULL) {
    Serial.println("Queue creation failed (out of heap)");
    return;
  }

  xTaskCreate(TaskReadSensor, "Read Sensor", 2048, NULL, 5, NULL);
  xTaskCreate(TaskPrintQueue, "Print Queue", 2048, NULL, 2, NULL);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}