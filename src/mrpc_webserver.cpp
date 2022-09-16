#include "mrpc.h"

#ifdef ARDUINO_ARCH_ESP32
#include <WebServer.h>
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266HTTPUpdateServer.h>
ESP8266HTTPUpdateServer httpUpdater;
#endif

using namespace MRPC;

#ifdef ARDUINO_ARCH_ESP32
WebServer _webserver(80);
#endif
#ifdef ARDUINO_ARCH_ESP8266
ESP8266WebServer _webserver(80);
#endif

void handleRoot() {
    Serial.println("Scanning networks");
    int wifi_count = WiFi.scanNetworks();
    Serial.println("Scan done");
    String response =
    "<form action=\"/connect\" method=\"get\">\
    <label>SSID:</label>\
    <select name=\"ssid\">";

    for(int i = 0; i < wifi_count; i++) {
        response += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>\n";
    }
    response +=
    "    </select>\
    <label>Password:</label>\
    <input type=\"password\" name=\"password\">\
    <BR>\
    <label>Mesh SSID:</label>\
    <input type=\"text\" name=\"mesh_ssid\">\
    <label>Mesh Password:</label>\
    <input type=\"text\" name=\"mesh_password\">\
    <input type=\"submit\" value=\"Connect\">\
    </form>";
    _webserver.send(200, "text/html", response);
}

void handleReset() {
    settings() = *new Json::Object();
    save_settings();
    ESP.restart();
}

void handleConnect() {
    if(_webserver.hasArg("ssid") && _webserver.hasArg("password")) {
        Json::Object &wifi_settings = settings()["wifi"].asObject();
        wifi_settings["ssid"] = _webserver.arg("ssid");
        wifi_settings["password"] = _webserver.arg("password");
        wifi_settings["mesh_ssid"] = _webserver.arg("mesh_ssid");
        wifi_settings["mesh_password"] = _webserver.arg("mesh_password");
        save_settings();
        _webserver.send(200, "text/html", "Successfully saved settings");
        ESP.restart();
    }
    else
        _webserver.send(500, "text/html", "Missing ssid or password");
}

void MRPCWebServer::init() {
    _webserver.on("/", HTTP_GET, handleRoot);
    _webserver.on("/connect", HTTP_GET, handleConnect);
    _webserver.on("/reset", HTTP_GET, handleReset);


    #ifdef ARDUINO_ARCH_ESP8266
    httpUpdater.setup(&_webserver);
    #endif
    _webserver.begin();
}

void MRPCWebServer::poll() {
    _webserver.handleClient();
}