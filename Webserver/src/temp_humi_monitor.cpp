#include "temp_humi_monitor.h"
#include <math.h>

#define I2C_SDA         11
#define I2C_SCL         12
#define READ_PERIOD_MS  5000

DHT20 dht20;

// Reads the DHT20 periodically and publishes the values to the shared globals.
// A failed read stores NAN so consumers (web page, CoreIOT) can skip stale data.
void temp_humi_monitor(void *pvParameters) {
    Wire.begin(I2C_SDA, I2C_SCL);
    dht20.begin();
    vTaskDelay(pdMS_TO_TICKS(1000));   // DHT20 rejects reads within 1 s of power-up (status -15)
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

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(READ_PERIOD_MS));
    }
}
