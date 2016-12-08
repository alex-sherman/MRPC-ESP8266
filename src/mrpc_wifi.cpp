#include "mrpc_wifi.h"

using namespace MRPC;


void MRPCWifi::connect(Json::Object &wifi_settings) {
    if(wifi_settings["ssid"].isString()) {
        Serial.print("Connecting to ");
        Serial.println(wifi_settings["ssid"].asString());
        WiFi.begin(wifi_settings["ssid"].asString(), wifi_settings["password"].asString());
        for(int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
            delay(500);
            Serial.print(".");
        }
        Serial.println();
    }
    //TODO: Attempt to connect to mesh

    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("Connection successful");
        if(wifi_settings["mesh_ssid"].isString()) {
            Serial.println("Starting mesh AP");
            WiFi.softAPConfig(IPAddress(192, 168, 2, 1), IPAddress(192, 168, 2, 1), IPAddress(255, 255, 255, 0));
            WiFi.softAP(wifi_settings["mesh_ssid"].asString(), wifi_settings["mesh_password"].asString());
        }
    }
    else {
        Serial.println("Connection failed");
        setupWiFiAP("password");
    }
}

void MRPCWifi::setupWiFiAP(const char* password) {

    // Do a little work to get a unique-ish name. Append the
    // last two bytes of the MAC (HEX'd) to "Thing-":
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
    String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
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