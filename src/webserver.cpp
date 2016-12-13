#include "mrpc.h"

using namespace MRPC;

static ESP8266WebServer webserver(80);

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
    webserver.send(200, "text/html", response);
}

void handleReset() {
    settings() = *new Json::Object();
    save_settings();
    ESP.restart();
}

void handleConnect() {
    if(webserver.hasArg("ssid") && webserver.hasArg("password")) {
        Json::Object &wifi_settings = settings()["wifi"].asObject();
        wifi_settings["ssid"] = webserver.arg("ssid");
        wifi_settings["password"] = webserver.arg("password");
        wifi_settings["mesh_ssid"] = webserver.arg("mesh_ssid");
        wifi_settings["mesh_password"] = webserver.arg("mesh_password");
        save_settings();
        webserver.send(200, "text/html", "Successfully saved settings");
        ESP.restart();
    }
    else
        webserver.send(500, "text/html", "Missing ssid or password");
}

void initWebserver() {
    webserver.on("/", HTTP_GET, handleRoot);
    webserver.on("/connect", HTTP_GET, handleConnect);
    webserver.on("/reset", HTTP_GET, handleReset);
    webserver.begin();
}

void pollWebserver() {
    webserver.handleClient();
}