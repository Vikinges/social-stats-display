#include "web_interface.h"
#include <ESP8266WebServer.h>
#include "storage.h"

#include <Updater.h>

ESP8266WebServer server(80);

String htmlForm(const Settings &settings) {
    String html = "<html><head><title>Setup</title>"
        "<style>body{font-family:sans-serif;background:#f4f4f4;margin:0;padding:0;}"
        ".container{max-width:400px;margin:40px auto;background:#fff;padding:24px 32px 32px 32px;border-radius:12px;box-shadow:0 2px 12px #0002;}"
        "h2{margin-top:0;}label{display:block;margin:12px 0 4px;}input,select{width:100%;padding:8px;margin-bottom:12px;border-radius:4px;border:1px solid #ccc;}button,input[type=submit]{background:#1976d2;color:#fff;border:none;padding:10px 0;border-radius:4px;font-size:1em;cursor:pointer;width:100%;margin-top:8px;}button:hover,input[type=submit]:hover{background:#1565c0;}hr{margin:32px 0 24px;}</style></head><body><div class='container'>";
    html += "<h2>Wi-Fi and API Setup</h2>";
    html += "<form method='POST' action='/save'>";
    int n = WiFi.scanNetworks();
    html += "<label>WiFi SSID:</label><select name='ssid'>";
    for (int i = 0; i < n; ++i) {
        String ssid = WiFi.SSID(i);
        html += "<option value='" + ssid + "'";
        if (ssid == settings.ssid) html += " selected";
        html += ">" + ssid + "</option>";
    }
    html += "<option value=''";
    if (settings.ssid == "") html += " selected";
    html += ">Other...</option></select>";
    html += "<input name='ssid' placeholder='Enter SSID' value='" + settings.ssid + "'>";
    html += "<label>WiFi Password:</label><input name='password' type='password' value='" + settings.password + "'>";
    String apiDefault = settings.apiUrl.length() ? settings.apiUrl : "https://linart.club/youtube.php";
    html += "<label>API URL:</label><input name='apiUrl' value='" + apiDefault + "'>";
    html += "<input type='submit' value='Save'></form>";
    html += "<hr><h2>OTA Update</h2>";
    html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
    html += "<input type='file' name='update'><input type='submit' value='Upload firmware'></form>";
    html += "</div></body></html>";
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

void handleRoot() {
    Settings s;
    loadSettings(s);
    server.send(200, "text/html", htmlForm(s));
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

// Captive portal: перенаправление всех HTTP-запросов на страницу конфигурации
#include <ESP8266WebServer.h>
#include <DNSServer.h>
DNSServer dnsServer;

void handleCaptive() { server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true); server.send(302, "text/plain", ""); }

void handleWebRequests(ESP8266WebServer &server, Settings &settings) {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    setupOTA(server);
    // Captive portal: перенаправление всех путей на /
    server.onNotFound(handleCaptive);
}

void startWebInterface() {
    Settings s;
    loadSettings(s);
    handleWebRequests(server, s);
    server.begin();
    // Captive portal DNS: перенаправлять все DNS-запросы на ESP8266
    dnsServer.start(53, "", WiFi.softAPIP());
    while (true) {
        dnsServer.processNextRequest();
        server.handleClient();
        delay(1);
    }
}
