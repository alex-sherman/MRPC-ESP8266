#include "mrpc_wifi.h"

#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#endif

using namespace MRPC;

IPAddress MRPCWifi::client_netmask = IPAddress(255,255,255,255);
IPAddress MRPCWifi::client_addr = IPAddress(0,0,0,0);
IPAddress MRPCWifi::ap_netmask = IPAddress(255,255,255,255);
IPAddress MRPCWifi::ap_addr = IPAddress(0,0,0,0);

bool connection_in_progress = false;
bool connected = false;

void _connect(const char *ssid, const char *password) {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    connection_in_progress = true;
}

int match_ssid(String ssid) {
    int n = WiFi.scanComplete();
    for(int i = 0; i < n; i++) {
        if(WiFi.SSID(i) == ssid)
            return i;
    }
    return -1;
}

void MRPCWifi::_onScanDone(Json::Object &wifi_settings) {
    failures ++;
    if(wifi_settings["ssid"].isString()) {
        if(match_ssid(wifi_settings["ssid"].asString()) >= 0) {
            _connect(wifi_settings["ssid"].asString(), wifi_settings["password"].asString());
            return;
        }
        Serial.println("No matching SSID found");
    }
    if(wifi_settings["mesh_ssid"].isString()) {
        if(match_ssid(wifi_settings["mesh_ssid"].asString()) >= 0) {
            _connect(wifi_settings["mesh_ssid"].asString(), wifi_settings["mesh_password"].asString());
            return;
        }
    }
    WiFi.scanNetworks(true);
}

void MRPCWifi::_onDisconnect() {
    Serial.println("Disconnected");
    WiFi.softAPdisconnect(true); 
}

void MRPCWifi::_onConnect(Json::Object &wifi_settings) {
    Serial.println("Connected successfuly");
    failures = 0;
    WiFi.softAPdisconnect(true);
    client_addr = WiFi.localIP();
    client_netmask = WiFi.subnetMask();
    ap_addr = IPAddress(192, 168, WiFi.localIP()[2] + 1, 1);
    ap_netmask = IPAddress(255, 255, 255, 0);
    if(wifi_settings["mesh_ssid"].isString()) {
        WiFi.softAPConfig(ap_addr, ap_addr, ap_netmask);
        WiFi.softAP(wifi_settings["mesh_ssid"].asString(), wifi_settings["mesh_password"].asString());
    }
}

void MRPCWifi::init() {
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
}

void MRPCWifi::poll() {
    Json::Object &wifi_settings = settings()["wifi"].asObject();
    if ((!wifi_settings["ssid"].isString() || failures > 5) && (WiFi.getMode() & WIFI_AP) == 0) {
        setupWiFiAP("password");
    }
    if (WiFi.status() == WL_CONNECTED) {
        connection_in_progress = false;
        if(!connected) {
            _onConnect(wifi_settings);
            connected = true;
        }
    } else {
        if(connected) _onDisconnect();
        connected = false;
        if(!connection_in_progress) {
            if(WiFi.scanComplete() > 0) {
                _onScanDone(wifi_settings);
            } else {
                WiFi.scanNetworks(true);
            }
        }
    }
}

IPAddress MRPCWifi::localIP() {
    return WiFi.localIP();
}

void MRPCWifi::setupWiFiAP(const char* password) {
    String macID = String(&guid().chars[36-4]);
    macID.toUpperCase();
    String AP_NameString = "MRPC " + macID;
    Serial.println("Creating AP");

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (int i=0; i<AP_NameString.length(); i++)
        AP_NameChar[i] = AP_NameString.charAt(i);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
    if (!WiFi.softAP(AP_NameChar, password))
        Serial.println("Failed to create AP");
}