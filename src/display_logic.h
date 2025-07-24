#pragma once

#include <Arduino.h>

void showStats(uint32_t subs, uint32_t views, const String &status = "");
void showProgressBar(uint8_t percent);
void showMessage(const String &msg, const String &status = "");
void showProgressAnimation(const String &status = "");
