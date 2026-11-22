// Publishes sensor telemetry to CoreIOT over MQTT while the board is on a router (STA).
// In AP mode the task blocks on xBinarySemaphoreInternet, which the web server task
// gives after a successful router connection.
#include "coreiot_task.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <math.h>
#include "secrets.h"

#define COREIOT_HOST     "app.coreiot.io"
#define COREIOT_PORT     1883
#define TOPIC_TELEMETRY  "v1/devices/me/telemetry"
#define PUBLISH_MS       5000
#define RETRY_MS         5000

volatile bool isCoreIOTConnected = false;

static WiFiClient netClient;
static PubSubClient mqtt(netClient);

// No light sensor on this board: simulated as a bounded random walk (300-500 lux).
static float simulatedLightLux()
{
  static float lux = 400.0f;
  lux += (float)random(-150, 151) / 10.0f;
  lux = constrain(lux, 300.0f, 500.0f);
  return lux;
}

static bool wifiReady()
{
  return isWifiConnected && WiFi.status() == WL_CONNECTED;
}

void coreiot_task(void *pvParameters)
{
  mqtt.setServer(COREIOT_HOST, COREIOT_PORT);
  uint32_t lastPublish = 0;

  while (true)
  {
    // No router link: drop the MQTT session and wait for the web server's signal
    if (!wifiReady())
    {
      if (mqtt.connected())
      {
        mqtt.disconnect();
        Serial.println("CoreIOT: Wi-Fi lost, MQTT disconnected");
      }
      isCoreIOTConnected = false;
      xSemaphoreTake(xBinarySemaphoreInternet, pdMS_TO_TICKS(5000));
      continue;
    }

    // Router link up, MQTT session down: (re)connect
    if (!mqtt.connected())
    {
      isCoreIOTConnected = false;
      Serial.print("CoreIOT: connecting... ");
      if (mqtt.connect(COREIOT_CLIENT_ID, COREIOT_USERNAME, COREIOT_PASSWORD))
      {
        Serial.println("connected");
        isCoreIOTConnected = true;
        lastPublish = 0;                         // publish immediately
      }
      else
      {
        // 4 = bad credentials, 5 = not authorized, -2 = network/DNS failure
        Serial.printf("failed (state = %d), retry in %d s\n", mqtt.state(), RETRY_MS / 1000);
        vTaskDelay(pdMS_TO_TICKS(RETRY_MS));
        continue;
      }
    }

    mqtt.loop();

    // Periodic telemetry
    if (lastPublish == 0 || millis() - lastPublish >= PUBLISH_MS)
    {
      lastPublish = millis();
      portENTER_CRITICAL(&sensor_mux);
      float t = glob_temperature;
      float h = glob_humidity;
      portEXIT_CRITICAL(&sensor_mux);

      if (isfinite(t) && isfinite(h))           // skip while the sensor is in error
      {
        char payload[96];
        snprintf(payload, sizeof(payload),
                 "{\"temperature\":%.2f,\"humidity\":%.2f,\"light\":%.0f}",
                 t, h, simulatedLightLux());
        bool ok = mqtt.publish(TOPIC_TELEMETRY, payload);
        Serial.printf("CoreIOT: %s %s\n", ok ? "published" : "publish FAILED", payload);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
