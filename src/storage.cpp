#include "storage.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#define SETTINGS_FILE "/settings.json"

bool loadSettings(Settings &settings) {
    if (!LittleFS.begin()) return false;
    File f = LittleFS.open(SETTINGS_FILE, "r");
    if (!f) return false;
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    if (err) return false;
    settings.ssid = doc["ssid"] | "";
    settings.password = doc["password"] | "";
    settings.apiUrl = doc["apiUrl"] | "";
    return true;
}

bool saveSettings(const Settings &settings) {
    if (!LittleFS.begin()) return false;
    File f = LittleFS.open(SETTINGS_FILE, "w");
    if (!f) return false;
    JsonDocument doc;
    doc["ssid"] = settings.ssid;
    doc["password"] = settings.password;
    doc["apiUrl"] = settings.apiUrl;
    serializeJson(doc, f);
    f.close();
    return true;
}

void resetSettings() {
    if (!LittleFS.begin()) return;
    LittleFS.remove(SETTINGS_FILE);
}

