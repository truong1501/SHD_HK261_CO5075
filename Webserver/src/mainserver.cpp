#include "mainserver.h"
#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include "wifi_credentials.h"
#include "web_pages.h"
#include "logo_png.h"

bool led1_state = false;
bool led2_state = false;
bool isAPMode = true;

WebServer server(80);

unsigned long connect_start_ms = 0;
bool connecting = false;
bool saveCredentialsOnConnect = false;
static bool scanPending = false;
static bool scanStartedInAP = false;
static bool scanRunning = false;   // The driver accepted the scan request.
static unsigned long scanStartMs = 0;
static unsigned long scanLastTryMs = 0;
static const unsigned long SCAN_RETRY_MS = 200;
// Arduino's scanComplete() gives up after 6 s, but with the AP active the radio
// returns to the AP channel between channels and a full scan can take longer.
static const unsigned long SCAN_TIMEOUT_MS = 15000;
static const int MAX_SCAN_RESULTS = 6;
static String scanResponse = "{\"status\":\"idle\",\"networks\":[]}";

// SSIDs can contain quotes and control characters; escape them for JSON.
static String quoteJson(const String &value)
{
  const char hex[] = "0123456789abcdef";
  String result = "\"";
  for (size_t i = 0; i < value.length(); ++i)
  {
    unsigned char c = value[i];
    if (c == '"' || c == '\\')
    {
      result += '\\';
      result += static_cast<char>(c);
    }
    else if (c < 0x20)
    {
      result += "\\u00";
      result += hex[c >> 4];
      result += hex[c & 0x0f];
    }
    else
    {
      result += static_cast<char>(c);
    }
  }
  result += '"';
  return result;
}

static void finishWiFiScan(int16_t count)
{
  if (count < 0)
  {
    scanResponse = "{\"status\":\"error\",\"networks\":[]}";
  }
  else
  {
    // Keep only the strongest networks, one entry per SSID, sorted by RSSI.
    int best[MAX_SCAN_RESULTS];
    int found = 0;
    for (int i = 0; i < count; ++i)
    {
      if (WiFi.SSID(i).isEmpty()) continue;
      int rssi = WiFi.RSSI(i);
      int pos = -1;
      for (int j = 0; j < found; ++j)
      {
        if (WiFi.SSID(best[j]) == WiFi.SSID(i)) { pos = j; break; }
      }
      if (pos >= 0)
      {
        if (rssi <= WiFi.RSSI(best[pos])) continue; // Weaker AP of a listed SSID.
      }
      else if (found < MAX_SCAN_RESULTS)
      {
        pos = found++;
      }
      else if (rssi > WiFi.RSSI(best[found - 1]))
      {
        pos = found - 1; // Replace the weakest entry.
      }
      else
      {
        continue;
      }
      // Move the entry up while it is stronger than the one before it.
      while (pos > 0 && rssi > WiFi.RSSI(best[pos - 1]))
      {
        best[pos] = best[pos - 1];
        --pos;
      }
      best[pos] = i;
    }

    scanResponse = "{\"status\":\"done\",\"networks\":[";
    for (int j = 0; j < found; ++j)
    {
      int i = best[j];
      if (j) scanResponse += ',';
      scanResponse += "{\"ssid\":" + quoteJson(WiFi.SSID(i)) +
                      ",\"rssi\":" + String(WiFi.RSSI(i)) +
                      ",\"secure\":" +
                      String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false") + "}";
    }
    scanResponse += "]}";
  }
  WiFi.scanDelete();
  scanPending = false;
  if (scanStartedInAP && isAPMode) WiFi.mode(WIFI_AP);
}

static void updateWiFiScan()
{
  if (!scanPending) return;
  if (millis() - scanStartMs > SCAN_TIMEOUT_MS)
  {
    Serial.println(scanRunning ? "Wi-Fi scan timed out" : "Wi-Fi scan could not start");
    finishWiFiScan(WIFI_SCAN_FAILED);
    return;
  }
  if (!scanRunning)
  {
    // Right after switching to AP_STA the STA interface is still starting and
    // the driver rejects the scan; retry instead of reporting an error.
    if (millis() - scanLastTryMs >= SCAN_RETRY_MS)
    {
      scanLastTryMs = millis();
      scanRunning = WiFi.scanNetworks(true) == WIFI_SCAN_RUNNING;
    }
    return;
  }
  int16_t count = WiFi.scanComplete();
  // WIFI_SCAN_FAILED may only mean Arduino's own 6 s limit passed; wait for our limit.
  if (count >= 0) finishWiFiScan(count);
}

// ========== Handlers ==========
void handleRoot() { server.send_P(200, "text/html; charset=utf-8", DASHBOARD_HTML); }

void handleSettings() { server.send_P(200, "text/html; charset=utf-8", SETTINGS_HTML); }

// Static assets rarely change; let the browser cache them instead of re-downloading.
void handleStyle()
{
  server.sendHeader("Cache-Control", "public, max-age=86400");
  server.send_P(200, "text/css", STYLE_CSS);
}

void handleLogo()
{
  server.sendHeader("Cache-Control", "public, max-age=86400");
  server.send_P(200, "image/png", reinterpret_cast<PGM_P>(LOGO_PNG), sizeof(LOGO_PNG));
}

void handleToggle()
{
  int led = server.arg("led").toInt();
  if (led == 1)
  {
    led1_state = !led1_state;
    digitalWrite(LED1_PIN, led1_state ? HIGH : LOW);
  }
  else if (led == 2)
  {
    led2_state = !led2_state;
    digitalWrite(LED2_PIN, led2_state ? HIGH : LOW);
  }
  server.send(200, "application/json",
              "{\"led1\":\"" + String(led1_state ? "ON" : "OFF") +
                  "\",\"led2\":\"" + String(led2_state ? "ON" : "OFF") + "\"}");
}

void handleSensors()
{
  portENTER_CRITICAL(&sensor_mux);
  float t = glob_temperature;
  float h = glob_humidity;
  portEXIT_CRITICAL(&sensor_mux);
  String json = "{\"temp\":" + (isfinite(t) ? String(t) : String("null")) +
                ",\"hum\":" + (isfinite(h) ? String(h) : String("null")) +
                ",\"led1\":\"" + String(led1_state ? "ON" : "OFF") +
                "\",\"led2\":\"" + String(led2_state ? "ON" : "OFF") + "\"";
  if (isAPMode)
  {
    json += ",\"mode\":\"AP\",\"ip\":\"" + WiFi.softAPIP().toString() + "\"";
  }
  else if (WiFi.status() == WL_CONNECTED)
  {
    // RSSI of the router link, shown on the dashboard.
    json += ",\"mode\":\"STA\",\"ip\":\"" + WiFi.localIP().toString() +
            "\",\"rssi\":" + String(WiFi.RSSI());
  }
  else
  {
    json += ",\"mode\":\"STA\",\"ip\":\"--\"";
  }
  json += "}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

void handleScanStart()
{
  if (connecting || scanPending)
  {
    server.send(409, "text/plain", "Wi-Fi is busy; try again shortly");
    return;
  }
  scanStartedInAP = isAPMode;
  // Scanning needs STA enabled; AP_STA keeps the settings page accessible.
  if (isAPMode && !WiFi.mode(WIFI_AP_STA))
  {
    server.send(503, "text/plain", "Cannot start Wi-Fi scan");
    return;
  }
  scanResponse = "{\"status\":\"scanning\",\"networks\":[]}";
  scanPending = true;
  scanRunning = false;
  scanStartMs = millis();
  scanLastTryMs = scanStartMs - SCAN_RETRY_MS; // Try to start immediately.
  updateWiFiScan();
  server.sendHeader("Cache-Control", "no-store");
  server.send(202, "application/json", scanResponse);
}

void handleScanResult()
{
  updateWiFiScan();
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", scanResponse);
}

void handleConnect()
{
  if (scanPending || connecting)
  {
    server.send(409, "text/plain", "Wi-Fi is busy; wait for scanning or connecting to finish");
    return;
  }
  String routerSsid = server.arg("ssid");
  String routerPassword = server.arg("pass");
  if (routerSsid.isEmpty() || routerSsid.length() > 32 || routerPassword.length() > 64)
  {
    server.send(400, "text/plain", "Invalid SSID or password length");
    return;
  }

  wifi_ssid = routerSsid;
  wifi_password = routerPassword;
  // Save only after a successful connection, preserving the previous config on failure.
  saveCredentialsOnConnect = true;
  server.send(200, "text/plain", "Connecting....");
  connectToWiFi();
}

// ========== WiFi ==========
void setupServer()
{
  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/sensors", HTTP_GET, handleSensors);
  server.on("/settings", HTTP_GET, handleSettings);
  server.on("/style.css", HTTP_GET, handleStyle);
  server.on("/logo.png", HTTP_GET, handleLogo);
  server.on("/connect", HTTP_POST, handleConnect);
  server.on("/scan", HTTP_POST, handleScanStart);
  server.on("/scan", HTTP_GET, handleScanResult);
  server.begin();
}

void startAP()
{
  WiFi.mode(WIFI_AP);
  isAPMode = WiFi.softAP(ssid.c_str(), password.c_str());
  connecting = false;
  isWifiConnected = false;
  saveCredentialsOnConnect = false;

  if (!isAPMode)
  {
    Serial.println("softAP failed: check AP password (at least 8 characters)");
    return;
  }

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void connectToWiFi()
{
  isAPMode = false;
  connecting = true;
  connect_start_ms = millis();
  isWifiConnected = false;
  WiFi.mode(WIFI_STA);
  if (wifi_password.isEmpty())
  {
    WiFi.begin(wifi_ssid.c_str());
  }
  else
  {
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  }
  Serial.print("Connecting to: ");
  Serial.println(wifi_ssid);
}

// ========== Main task ==========
void main_server_task(void *pvParameters)
{
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  pinMode(BOOT_PIN, INPUT_PULLUP);

  // Preferences manages persistence; avoid implicit credential writes by WiFi.begin().
  WiFi.persistent(false);
  if (digitalRead(BOOT_PIN) != LOW && loadWiFiCredentials(wifi_ssid, wifi_password))
  {
    Serial.println("Trying saved router Wi-Fi");
    connectToWiFi();
  }
  else
  {
    startAP();
  }
  // Register routes once; the server remains active across AP/STA changes.
  setupServer();

  while (1)
  {
    server.handleClient();
    updateWiFiScan();

    // BOOT Button to switch to AP Mode
    if (digitalRead(BOOT_PIN) == LOW)
    {
      vTaskDelay(pdMS_TO_TICKS(100));
      if (digitalRead(BOOT_PIN) == LOW)
      {
        if (!isAPMode)
        {
          startAP();
        }
      }
    }

    // STA Mode
    if (connecting)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.print("STA IP address: ");
        Serial.println(WiFi.localIP());
        isWifiConnected = true; // Connected to router; Internet not verified.

        if (saveCredentialsOnConnect)
        {
          if (saveWiFiCredentials(wifi_ssid, wifi_password))
          {
            Serial.println("Router Wi-Fi saved to flash");
          }
          else
          {
            Serial.println("Connected, but failed to save router Wi-Fi to flash");
          }
          saveCredentialsOnConnect = false;
        }

        xSemaphoreGive(xBinarySemaphoreInternet);

        isAPMode = false;
        connecting = false;
      }
      else if (millis() - connect_start_ms > 10000)
      { // timeout 10s
        Serial.println("WiFi connect failed! Back to AP.");
        startAP();
      }
    }

    vTaskDelay(pdMS_TO_TICKS(20)); // avoid watchdog reset
  }
}
