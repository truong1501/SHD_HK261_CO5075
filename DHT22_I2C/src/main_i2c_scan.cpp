#include <Arduino.h>
#include <Wire.h>
#include <DHT20.h>             // thư viện robtillaart/DHT20 (lib_deps)

#define I2C_SDA         11     // GPIO11
#define I2C_SCL         12     // GPIO12
#define I2C_FREQ_HZ     100000 // I2C standard-mode
#define DHT20_ADDR      0x38
#define READ_PERIOD_MS  5000   // DHT20 cần >= 1 s giữa hai lần đo

DHT20 dht20(&Wire);

// Quét toàn bộ địa chỉ 7-bit hợp lệ 0x08..0x77
void i2c_scan() {
  Serial.println("Scanning I2C bus...");
  uint8_t found = 0;
  for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {          // 0 = thiết bị trả ACK
      Serial.printf("  Found device at 0x%02X%s\n", addr,
                    addr == DHT20_ADDR ? "  <- DHT20" : "");
      found++;
    }
  }
  Serial.printf("Scan done: %u device(s)\n", found);
}

// Task đọc cảm biến định kỳ, có xử lý lỗi
void task_dht20(void *pvParameters) {
  TickType_t lastWake = xTaskGetTickCount();
  while (true) {
    int status = dht20.read();                    // đọc và kiểm tra CRC
    if (status == DHT20_OK) {
      Serial.printf("Temperature: %.2f C | Humidity: %.2f %%\n",
                    dht20.getTemperature(), dht20.getHumidity());
    } else {
      // In mã lỗi: -10 CRC sai, -11 mất kết nối, -14 timeout...
      Serial.printf("DHT20 read error (status = %d)\n", status);
      if (status == DHT20_ERROR_CONNECT) {
        dht20.begin();                            // thử khởi tạo lại cảm biến
      }
    }
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(READ_PERIOD_MS));
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ_HZ);     // khởi tạo bus I2C trên GPIO11/12
  i2c_scan();
  if (!dht20.begin()) {
    Serial.println("DHT20 not found at 0x38, check wiring!");
  }
  xTaskCreate(task_dht20, "DHT20", 4096, NULL, 2, NULL);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
