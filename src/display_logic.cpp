// Исправления: добавлены необходимые include и extern
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "display_logic.h"

extern Adafruit_SSD1306 display;
extern bool serialMirrorEnabled;

void showMessage(const String &msg, const String &status) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(2, 0);
    display.print(status);
    if (serialMirrorEnabled) Serial.println(status);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println(msg);
    if (serialMirrorEnabled) Serial.println(msg);
    display.display();
}

void showProgressAnimation(const String &status) {
    for (uint8_t i = 0; i <= 100; i += 10) {
        display.clearDisplay();
        int statusX = 2 + (i % 40);
        display.setTextSize(1);
        display.setCursor(statusX, 0);
        display.print(status);
        if (serialMirrorEnabled) Serial.println(status);
        display.setTextSize(1);
        display.setCursor(32, 24);
        display.print("Updating...");
        if (serialMirrorEnabled) Serial.println("Updating...");
        // Прогресс-бар
        display.fillRect(0, 56, i * 128 / 100, 8, SSD1306_WHITE);
        display.drawRect(0, 56, 128, 8, SSD1306_WHITE);
        // Логотип YouTube
        int iconX = 104;
        int iconY = 32;
        int iconW = 16;
        int iconH = 12;
        int triX1 = iconX + 4;
        int triY1 = iconY + 3;
        int triX2 = iconX + iconW - 4;
        int triY2 = iconY + iconH / 2;
        int triX3 = iconX + 4;
        int triY3 = iconY + iconH - 3;
        if (i % 2 == 0) {
            display.fillRect(iconX, iconY, iconW, iconH, SSD1306_BLACK);
            display.fillTriangle(triX1, triY1, triX2, triY2, triX3, triY3, SSD1306_WHITE);
        } else {
            display.fillRect(iconX, iconY, iconW, iconH, SSD1306_BLACK);
            display.fillRoundRect(iconX, iconY, iconW, iconH, 3, SSD1306_WHITE);
            display.fillTriangle(triX1, triY1, triX2, triY2, triX3, triY3, SSD1306_BLACK);
        }
        display.display();
        delay(60);
    }
}
#include "display_logic.h"

#include <Adafruit_SSD1306.h>

// Глобальный флаг для дублирования вывода в Serial
bool serialMirrorEnabled = true;

extern Adafruit_SSD1306 display;

void showStats(uint32_t subs, uint32_t views, int subsToday, int viewsToday, const String &status) {
    String subsStr = String(subs);
    String viewsStr = String(views);
    for (uint8_t step = 0; step < 40; step++) {
        display.clearDisplay();
        display.fillRect(0, 0, 128, 12, SSD1306_BLACK);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(4, 2);
        display.print("i Youtube @LinArt");
        if (serialMirrorEnabled) Serial.println("i Youtube @LinArt");
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(8, 20);
        display.print("Subs: ");
        if (serialMirrorEnabled) Serial.print("Subs: ");
        int subsAnimX = 50 + (step % 10);
        display.setCursor(subsAnimX, 20);
        display.print(subsStr);
        if (serialMirrorEnabled) Serial.print(subsStr);
        Serial.println();
        display.setCursor(8, 36);
        display.print("Views: ");
        if (serialMirrorEnabled) Serial.print("Views: ");
        int viewsAnimX = 50 + (step % 10);
        display.setCursor(viewsAnimX, 36);
        display.print(viewsStr);
        if (serialMirrorEnabled) Serial.println(viewsStr);
        // Нижняя строка: прирост за 24ч
        display.setCursor(0, 54);
        display.print("24H S>");
        display.print(subsToday);
        display.print(" W>");
        display.print(viewsToday);
        if (serialMirrorEnabled) {
            Serial.print("24H S>"); Serial.print(subsToday); Serial.print(" W>"); Serial.println(viewsToday);
        }
        int iconX = 104;
        int iconY = 32;
        int iconW = 16;
        int iconH = 12;
        int triX1 = iconX + 4;
        int triY1 = iconY + 3;
        int triX2 = iconX + iconW - 4;
        int triY2 = iconY + iconH / 2;
        int triX3 = iconX + 4;
        int triY3 = iconY + iconH - 3;
        if (step % 2 == 0) {
            display.fillRect(iconX, iconY, iconW, iconH, SSD1306_BLACK);
            display.fillTriangle(triX1, triY1, triX2, triY2, triX3, triY3, SSD1306_WHITE);
        } else {
            display.fillRect(iconX, iconY, iconW, iconH, SSD1306_BLACK);
            display.fillRoundRect(iconX, iconY, iconW, iconH, 3, SSD1306_WHITE);
            display.fillTriangle(triX1, triY1, triX2, triY2, triX3, triY3, SSD1306_BLACK);
        }
        display.display();
        delay(50);
    }
}
