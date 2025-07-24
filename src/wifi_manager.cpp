#include "wifi_manager.h"
#include <ESP8266WiFi.h>

#include "storage.h"

extern Settings settings;

void startWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(settings.ssid.c_str(), settings.password.c_str());
    // Можно добавить ожидание подключения
    Serial.print("Connecting to SSID: ");
    Serial.println(settings.ssid);
    unsigned long startAttemptTime = millis();
    // Ждём максимум 10 секунд
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.print("WiFi failed, status: ");
        Serial.println(WiFi.status());
    }
}

void startAPMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Youtube_@LinArt", "12345678"); // SSID и пароль
}

bool isWiFiConfigured() {
    Serial.print("WiFi config check: SSID=");
    Serial.print(settings.ssid);
    Serial.print(" PASSWORD=");
    Serial.println(settings.password);
    return settings.ssid.length() > 0 && settings.password.length() > 0;
}
