

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "hardware_init.h"
#include "wifi_manager.h"
#include "web_interface.h"
#include "storage.h"
#include "display_logic.h"
#include "api_client.h"
#include "failover.h"

// Глобальные переменные
Settings settings;
uint8_t wifiAttempts = 0;
const uint8_t maxWifiAttempts = 3;
uint32_t subs = 0, views = 0;
bool apiOk = false;


void setup() {
  initHardware();
  showMessage("Loading...", "BOOT");

  if (!loadSettings(settings) || !isWiFiConfigured()) {
    // Нет настроек — сразу в режим AP и web-интерфейс
    showMessage("WiFi setup", "AP Mode");
    startAPMode();
    startWebInterface();
    return;
  }

  // Пробуем подключиться к Wi-Fi
  while (wifiAttempts < maxWifiAttempts) {
    showMessage("Connecting...", "WiFi");
    startWiFi();
    if (WiFi.status() == WL_CONNECTED) {
      showMessage("WiFi OK", "WiFi");
      break;
    }
    wifiAttempts++;
    delay(2000);
  }

  if (WiFi.status() != WL_CONNECTED) {
    // Не удалось подключиться — fallback в AP
    showMessage("WiFi FAIL", "AP Mode");
    handleFailover();
    startAPMode();
    startWebInterface();
    return;
  }

  // Wi-Fi подключен, пробуем получить статистику
  showProgressAnimation("API");
  for (uint8_t i = 0; i < 3; ++i) {
    apiOk = fetchYouTubeStats(subs, views);
    if (apiOk) break;
    delay(2000);
  }
  if (!apiOk) {
    showMessage("API FAIL", "API");
    handleFailover();
    startAPMode();
    startWebInterface();
    return;
  }

  // Всё ок — показываем статистику
  showStats(subs, views, "OK");
}


void loop() {
  static unsigned long lastUpdate = 0;
  static uint8_t failCount = 0;
  const uint8_t maxApiFails = 3;
  // Обработка web-интерфейса (OTA и настройка)
  yield();
  // Если в режиме AP/web-интерфейса — просто обслуживаем сервер
  // (web_interface.cpp: startWebInterface содержит свой цикл)

  // Если Wi-Fi подключен — обновляем статистику раз в 60 сек
  if (WiFi.status() == WL_CONNECTED) {
    if (millis() - lastUpdate > 60000) {
      showProgressAnimation("API");
      if (fetchYouTubeStats(subs, views)) {
        showStats(subs, views, "OK");
        failCount = 0;
      } else {
        failCount++;
        showMessage("API FAIL", "API");
        if (failCount >= maxApiFails) {
          handleFailover();
          startAPMode();
          startWebInterface();
          return;
        }
      }
      lastUpdate = millis();
    }
  }
  delay(10);
}