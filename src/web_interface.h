#pragma once
#include <ESP8266WebServer.h>
#include "storage.h"

void startWebInterface();
void handleWebRequests(ESP8266WebServer &server, Settings &settings);
void setupOTA(ESP8266WebServer &server);
