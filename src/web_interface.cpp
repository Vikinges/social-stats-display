#include "web_interface.h"
#include <ESP8266WebServer.h>
#include <Updater.h>
#include <DNSServer.h>
extern Settings settings;
extern DNSServer dnsServer;

ESP8266WebServer server(80);

extern uint32_t subs, views;
extern int subsToday, viewsToday;

String htmlForm(const Settings &settings) {
    int n = WiFi.scanNetworks();
    String apiDefault = "https://linart.club/youtube.php";
    String ssidDefault = settings.ssid.length() ? settings.ssid : "MyWiFi";
    String passwordDefault = settings.password.length() ? settings.password : "";
    String apiUrlDefault = settings.apiUrl.length() ? settings.apiUrl : apiDefault;
    String html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Device Setup</title><style>body{font-family:Arial,sans-serif;background:#f0f4fa;margin:0;}.container{max-width:400px;margin:40px auto;background:#fff;border-radius:12px;box-shadow:0 2px 8px rgba(0,0,0,0.08);padding:32px;}h2{color:#2a5dff;margin-bottom:24px;}label{display:block;margin-top:16px;color:#333;font-weight:bold;}select,input[type='text'],input[type='password'],input[type='url']{width:100%;padding:8px;margin-top:4px;border:1px solid #ccc;border-radius:6px;}input[type='submit']{background:#2a5dff;color:#fff;border:none;padding:10px 24px;border-radius:6px;margin-top:24px;cursor:pointer;font-size:1em;}input[type='submit']:hover{background:#1a3db8;}.section{margin-bottom:32px;}.divider{border-top:1px solid #eee;margin:32px 0;}</style></head><body><div class='container'><h2>WiFi & API Setup</h2><form method='POST' action='/save'>";
    html += "<label for='ssid_select'>WiFi SSID</label>";
    html += "<select id='ssid_select' name='ssid_select' onchange=\"document.getElementById('ssid').value=this.value;\" style='margin-bottom:8px;'>";
    html += "<option value=''";
    if (settings.ssid == "") html += " selected";
    html += ">Select network...</option>";
    for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        html += "<option value='" + ssid + "'";
        if (ssidDefault == ssid) html += " selected";
        html += ">" + ssid + "</option>";
    }
    html += "</select>";
    html += "<input type='text' id='ssid' name='ssid' placeholder='Enter SSID manually' value='" + ssidDefault + "'>";
    html += "<label for='password'>WiFi Password</label>";
    html += "<input type='password' id='password' name='password' value='" + passwordDefault + "' required placeholder='Enter WiFi password'>";
    html += "<label for='apiUrl'>API URL</label>";
    html += "<input type='url' id='apiUrl' name='apiUrl' value='" + apiUrlDefault + "' required placeholder='https://linart.club/youtube.php'>";
    html += "<input type='submit' value='Save'>";
    html += "</form><div class='divider'></div>";
    html += "<h2>Statistics</h2>";
    html += "<div><b>Subscribers:</b> " + String(subs) + " (" + (subsToday > 0 ? "+" + String(subsToday) : "0") + " today)</div>";
    html += "<div><b>Views:</b> " + String(views) + " (" + (viewsToday > 0 ? "+" + String(viewsToday) : "0") + " today)</div>";
    html += "<div class='divider'></div>";
    html += "<h2>OTA Update</h2><form method='POST' action='/update' enctype='multipart/form-data'>";
    html += "<input type='file' name='update' required><input type='submit' value='Upload firmware'></form>";
    html += "<div class='divider'></div><h2>Change AP Password</h2><form method='POST' action='/ap_password'>";
    html += "<label for='ap_password'>New AP Password</label>";
    html += "<input type='password' id='ap_password' name='ap_password' value='" + passwordDefault + "' required placeholder='Enter new AP password'>";
    html += "<input type='submit' value='Change AP Password'></form></div></body></html>";
    return html;
}

void handleUpdateUpload() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        Update.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            server.send(200, "text/html", "<h2>Update complete! Rebooting...</h2>");
            delay(1000);
            ESP.restart();
        } else {
            server.send(200, "text/html", "<h2>Update failed!</h2>");
        }
    }
}

void setupOTA(ESP8266WebServer &server) {
    server.on("/update", HTTP_POST, [&]() {
        server.send(200, "text/html", "<h2>Uploading...</h2>");
    }, handleUpdateUpload);
}

void handleApPassword() {
  if (server.hasArg("ap_password")) {
    String newPass = server.arg("ap_password");
    if (newPass.length() >= 8) {
      settings.password = newPass;
      saveSettings(settings);
      WiFi.softAP(settings.ssid.c_str(), settings.password.c_str());
      server.send(200, "text/html", "<html><body>AP password changed!<br><a href='/'>Back</a></body></html>");
      return;
    } else {
      server.send(200, "text/html", "<html><body>Password must be at least 8 characters.<br><a href='/'>Back</a></body></html>");
      return;
    }
  }
  server.send(200, "text/html", "<html><body>Error: No password provided.<br><a href='/'>Back</a></body></html>");
}

void handleSave() {
    Settings s;
    s.ssid = server.arg("ssid");
    s.password = server.arg("password");
    s.apiUrl = server.arg("apiUrl");
    saveSettings(s);
    server.send(200, "text/html", "<html><body><h2>Saved! Rebooting...</h2></body></html>");
    delay(1000);
    ESP.restart();
}

void handleCaptive() {
  String html = "<html><head><meta http-equiv='refresh' content='0; url=http://";
  html += WiFi.softAPIP().toString();
  html += "'/></head><body>If the page did not open automatically, <a href='http://";
  html += WiFi.softAPIP().toString();
  html += "'>click here</a>.</body></html>";
  server.send(200, "text/html", html);
}

void handleWebRequests(ESP8266WebServer &server, Settings &settings) {
    server.on("/", HTTP_GET, [&]() {
        server.send(200, "text/html", htmlForm(settings));
    });
    server.on("/save", HTTP_POST, handleSave);
    server.on("/ap_password", HTTP_POST, handleApPassword);
    setupOTA(server);
    // Captive portal: перенаправление всех путей на /
    server.onNotFound(handleCaptive);
}

void startWebInterface() {
    Settings s;
    loadSettings(s);
    handleWebRequests(server, s);
    server.begin();
    // Captive portal DNS: запускать только если ESP в режиме AP
    if (WiFi.getMode() == WIFI_AP) {
        dnsServer.start(53, "*", WiFi.softAPIP());
    }
    // Теперь обработка запросов будет из loop() main.cpp
}


void webInterfaceLoop() {
    server.handleClient();
}


