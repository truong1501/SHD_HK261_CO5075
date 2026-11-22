#include "temp_humi_monitor.h"
#include <math.h>

DHT20 dht20;

// Task đọc DHT20 mỗi 5 s và cập nhật biến dùng chung cho webserver.
// Sửa so với bản gốc:
//  - đọc lỗi thì ghi NAN (không phải -1) để trang web hiện "--" đúng như README;
//  - ghi biến dùng chung bên trong sensor_mux, khớp với handleSensors() đọc trong mux;
//  - dùng vTaskDelayUntil để chu kỳ đọc cố định.
void temp_humi_monitor(void *pvParameters) {
    Wire.begin(11, 12);           // SDA = GPIO11, SCL = GPIO12
    dht20.begin();
    TickType_t lastWake = xTaskGetTickCount();

    while (1) {
        int status = dht20.read();
        float temperature = NAN;
        float humidity = NAN;
        if (status == DHT20_OK) {
            temperature = dht20.getTemperature();
            humidity = dht20.getHumidity();
            Serial.printf("Humidity: %.2f %%  Temperature: %.2f C\n", humidity, temperature);
        } else {
            Serial.printf("DHT20 read error (status = %d)\n", status);
        }

        portENTER_CRITICAL(&sensor_mux);
        glob_temperature = temperature;
        glob_humidity = humidity;
        portEXIT_CRITICAL(&sensor_mux);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(5000));
    }
}
