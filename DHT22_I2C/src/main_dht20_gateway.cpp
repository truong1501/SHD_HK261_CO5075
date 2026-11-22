// Serial gateway firmware: one JSON line per DHT20 reading, no Wi-Fi on the board.
// gateway/serial_gateway.py on the host forwards each line to CoreIOT over MQTT.
// Non-JSON lines (scan results, errors) are logged by the gateway and not published.
#include <Arduino.h>
#include <Wire.h>
#include <DHT20.h>

#define I2C_SDA         11
#define I2C_SCL         12
#define I2C_FREQ_HZ     100000
#define READ_PERIOD_MS  5000
#define DHT20_ADDR      0x38

DHT20 dht20(&Wire);

void i2c_scan() {
  Serial.println("Scanning I2C bus...");
  uint8_t found = 0;
  for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  Found device at 0x%02X%s\n", addr,
                    addr == DHT20_ADDR ? "  <- DHT20" : "");
      found++;
    }
  }
  Serial.printf("Scan done: %u device(s)\n", found);
}

void task_dht20_json(void *pvParameters) {
  vTaskDelay(pdMS_TO_TICKS(1000));   // DHT20 rejects reads within 1 s of power-up (status -15)
  TickType_t lastWake = xTaskGetTickCount();
  while (true) {
    int status = dht20.read();
    if (status == DHT20_OK) {
      Serial.printf("{\"temperature\":%.2f,\"humidity\":%.2f}\n",
                    dht20.getTemperature(), dht20.getHumidity());
    } else {
      Serial.printf("DHT20 read error (status = %d)\n", status);
      if (status == DHT20_ERROR_CONNECT) dht20.begin();
    }
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(READ_PERIOD_MS));
  }
}

void setup() {
  Serial.begin(115200);
  // Wait up to 3 s for the host to open the port. To see the scan again, press RST
  // while the gateway is running; it reopens the port automatically.
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) delay(10);

  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ_HZ);
  i2c_scan();
  if (!dht20.begin()) Serial.println("DHT20 not found at 0x38, check wiring!");
  xTaskCreate(task_dht20_json, "DHT20_JSON", 4096, NULL, 2, NULL);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
