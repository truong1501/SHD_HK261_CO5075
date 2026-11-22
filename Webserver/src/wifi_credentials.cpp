#include "wifi_credentials.h"
#include <Preferences.h>
#include <cstring>

namespace {
// Store the SSID/password together in one NVS value to keep them paired.
struct Credentials {
    char ssid[33];
    char password[65];
};
}

bool loadWiFiCredentials(String &routerSsid, String &routerPassword) {
    Preferences preferences;
    if (!preferences.begin("router-wifi", true)) {
        return false; // Includes the first boot, before this namespace exists.
    }

    Credentials saved = {};
    bool loaded = preferences.getBytesLength("credentials") == sizeof(saved);
    if (loaded) {
        loaded = preferences.getBytes("credentials", &saved, sizeof(saved)) == sizeof(saved);
    }
    preferences.end();

    if (!loaded || saved.ssid[0] == '\0' ||
        std::memchr(saved.ssid, '\0', sizeof(saved.ssid)) == nullptr ||
        std::memchr(saved.password, '\0', sizeof(saved.password)) == nullptr) {
        return false;
    }

    routerSsid = saved.ssid;
    routerPassword = saved.password;
    return true;
}

bool saveWiFiCredentials(const String &routerSsid, const String &routerPassword) {
    if (routerSsid.isEmpty() || routerSsid.length() > 32 || routerPassword.length() > 64) {
        return false;
    }

    Credentials saved = {};
    std::memcpy(saved.ssid, routerSsid.c_str(), routerSsid.length());
    std::memcpy(saved.password, routerPassword.c_str(), routerPassword.length());

    Preferences preferences;
    if (!preferences.begin("router-wifi", false)) {
        return false;
    }
    bool stored = preferences.putBytes("credentials", &saved, sizeof(saved)) == sizeof(saved);
    preferences.end();
    return stored;
}
