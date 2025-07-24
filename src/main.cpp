


#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>

#include "hardware_init.h"
#include "wifi_manager.h"
#include "web_interface.h"
#include "storage.h"
#include "display_logic.h"
#include "api_client.h"
#include "failover.h"

DNSServer dnsServer;
const byte DNS_PORT = 53;

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
  // Обработка кнопки FLASH
  static unsigned long flashPressStart = 0;
  static bool flashCounting = false;
  static bool flashActionPending = false;
  static uint8_t prevMode = WIFI_STA; // 0: STA, 1: AP
  const uint8_t maxApiFails = 3;
  bool inAPMode = (WiFi.getMode() == WIFI_AP);

  if (digitalRead(0) == LOW) {
    if (!flashCounting) {
      flashPressStart = millis();
      flashCounting = true;
      prevMode = WiFi.getMode();
      flashActionPending = true;
    }
    unsigned long held = (millis() - flashPressStart) / 1000;
    if (inAPMode) {
      showMessage(String("AP Reset: ") + held + "s", "FLASH");
      if (held >= 5) {
        // Сброс настроек и перезагрузка
        showMessage("Resetting...", "DEFAULT");
        resetSettings();
        delay(500);
        ESP.restart();
        while (true) delay(10); // Ждём перезагрузки
      }
    } else {
      showMessage(String("Hold: ") + held + "s", "FLASH");
      if (held >= 5) {
        // Запуск режима конфигурации
        showMessage("Config mode", "AP");
        startAPMode();
        startWebInterface();
        while (digitalRead(0) == LOW) delay(10); // Ждём отпускания
        flashCounting = false;
        flashActionPending = false;
        return;
      }
    }
    delay(100);
    return;
  } else {
    if (flashCounting && flashActionPending) {
      // FLASH отпущена до истечения таймера
      // Возврат к предыдущему режиму/экрану
      if (prevMode == WIFI_STA) {
        showStats(subs, views, "OK");
      } else {
        showMessage("WiFi setup", "AP Mode");
      }
      flashCounting = false;
      flashActionPending = false;
    }
  }

  // Captive portal: обслуживаем DNS только в режиме AP
  if (WiFi.getMode() == WIFI_AP) {
    dnsServer.processNextRequest();
  }
  // Обработка web-интерфейса (OTA и настройка)
  webInterfaceLoop();
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