#pragma once

#include <Arduino.h>

struct Settings {
    String ssid;
    String password;
    String apiUrl;
};

bool loadSettings(Settings &settings);
bool saveSettings(const Settings &settings);
void resetSettings();
