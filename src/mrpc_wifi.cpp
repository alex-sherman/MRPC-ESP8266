#include "mrpc_wifi.h"

using namespace MRPC;

IPAddress MRPCWifi::client_netmask = IPAddress(255,255,255,255);
IPAddress MRPCWifi::client_addr = IPAddress(0,0,0,0);
IPAddress MRPCWifi::ap_netmask = IPAddress(255,255,255,255);
IPAddress MRPCWifi::ap_addr = IPAddress(0,0,0,0);

void _connect(const char *ssid, const char *password) {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    for(int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
}

void MRPCWifi::connect(Json::Object &wifi_settings) {
    MRPCWifi::client_netmask = IPAddress(255, 255, 255, 255);
    if(wifi_settings["ssid"].isString()) {
        _connect(wifi_settings["ssid"].asString(), wifi_settings["password"].asString());
        ap_addr = IPAddress(192, 168, 2, 1);
        ap_netmask = IPAddress(255, 255, 255, 0);
    }

    if(WiFi.status() != WL_CONNECTED && wifi_settings["mesh_ssid"].isString()) {
        WiFi.disconnect();
        Serial.print("Connecting to mesh ");
        Serial.println(wifi_settings["mesh_ssid"].asString());
        _connect(wifi_settings["mesh_ssid"].asString(), wifi_settings["mesh_password"].asString());
        if(WiFi.status() == WL_CONNECTED) {
            ap_addr = IPAddress(192, 168, WiFi.localIP()[2] + 1, 1);
            ap_netmask = IPAddress(255, 255, 255, 0);
        }
    }

    if(WiFi.status() == WL_CONNECTED) {
        client_addr = WiFi.localIP();
        client_netmask = WiFi.subnetMask();
        Serial.println("Connection successful");
        if(wifi_settings["mesh_ssid"].isString()) {
            Serial.println("Starting mesh AP");
            WiFi.softAPConfig(ap_addr, ap_addr, ap_netmask);
            WiFi.softAP(wifi_settings["mesh_ssid"].asString(), wifi_settings["mesh_password"].asString());
        }
    }
    else {
        Serial.println("Connection failed");
        setupWiFiAP("password");
    }
    Serial.println(ap_addr);
    Serial.println(ap_netmask);
    Serial.println(client_addr);
    Serial.println(client_netmask);
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
    WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_NameChar, password);
}