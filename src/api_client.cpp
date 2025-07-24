#include "api_client.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "storage.h"

extern Settings settings;

bool fetchYouTubeStats(uint32_t &subs, uint32_t &views) {
    if (!settings.apiUrl.length()) return false;
    WiFiClientSecure client;
    client.setInsecure();
    // Parse host and path
    String host, path = "/";
    int httpsIdx = settings.apiUrl.indexOf("https://");
    String url = settings.apiUrl;
    if (httpsIdx == 0) url = url.substring(8);
    int slashIdx = url.indexOf('/');
    if (slashIdx > 0) {
        host = url.substring(0, slashIdx);
        path = url.substring(slashIdx);
    } else {
        host = url;
    }
    if (!client.connect(host.c_str(), 443)) return false;
    String req = String("GET ") + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
    client.print(req);
    String payload;
    while (client.connected() || client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") break;
    }
    while (client.available()) {
        payload += client.readString();
    }
    client.stop();
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) return false;
    // Для формата {"subs":"2240","views":"1036877"}
    subs = String(doc["subs"]).toInt();
    views = String(doc["views"]).toInt();
    return true;
}

