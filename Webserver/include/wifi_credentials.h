#pragma once

#include <Arduino.h>

// Return false when no valid credentials exist or flash access fails.
bool loadWiFiCredentials(String &routerSsid, String &routerPassword);
bool saveWiFiCredentials(const String &routerSsid, const String &routerPassword);
