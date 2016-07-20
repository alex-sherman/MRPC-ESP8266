#include "mrpc.h"
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include "message.h"

using namespace MRPC;

Json::Object *eepromJSON;
ESP8266WebServer server(80);
void handleConnect();
void handleRoot();
void setupWiFiAP(const char*);
bool validWifiSettings();
int Message::id = 0;

void MRPC::init() {
    Message::id = 0;
    guid = UUID();
    routing = new Routing();
    register_service("/Routing", routing);

    EEPROM.begin(1024);
    char json[1024];
    EEPROM.get(0, json);
    Serial.print("EEPROM Contents: ");
    Serial.println(json);
    WiFi.mode(WIFI_STA);
    bool createAP = true;
    if(validWifiSettings()) {
        Serial.println("Found wifi settings:");
        Json::Object &wifi_settings = settings()["wifi"].asObject();
        Json::print(wifi_settings, Serial);
        Serial.println();

        WiFi.begin(wifi_settings["ssid"].asString(), wifi_settings["password"].asString());
        Serial.println("Connecting to WiFi");
        for(int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) {
            delay(500);
            Serial.print(".");
        }
        Serial.println();
        if(WiFi.status() == WL_CONNECTED) {
            Serial.println("Connection successful");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            createAP = false;
        }
        else
            Serial.println("Connection failed");
    }
    else {
        Serial.println("Couldn't find WiFi settings");
        settings()["wifi"] = *(new Json::Object());
        Json::println(settings(), Serial);
    }

    if(createAP) {
        Serial.println("Creating AP");
        setupWiFiAP("password");
    }
    Serial.println("Starting server");
    server.on("/", HTTP_GET, handleRoot);
    server.on("/connect", HTTP_GET, handleConnect);
    server.begin();
}

void MRPC::poll() {
    server.handleClient();

    bool output = false;
    for(auto &kvp : results) {
        if(kvp.valid && kvp.value.stale()) {
            kvp.valid = false;
            kvp.value.data.free_parsed();
        }
    }
    for(int i = 0; i < transports.size(); i++) {
        output |= transports[i]->poll();
    }
    for (auto &kvp : services)
    {
        kvp.value->update(millis());
    }
}

Json::Object &MRPC::settings() {
    if(eepromJSON == NULL) {
        char json[1024];
        EEPROM.get(0, json);
        Json::Value output = Json::parse(json);
        if(output.isObject()) {
            eepromJSON = &output.asObject();
        }
        else {
            eepromJSON = new Json::Object();
        }
    }
    return *eepromJSON;
}
void MRPC::save_settings() {
    char json[1024];
    Json::dump(settings(), json, sizeof(json));
    Serial.print("Saving to eeprom: ");
    Serial.println(json);
    EEPROM.put(0, json);
    EEPROM.commit();
}

void setupWiFiAP(const char *password)
{
    WiFi.mode(WIFI_AP);

// Do a little work to get a unique-ish name. Append the
// last two bytes of the MAC (HEX'd) to "Thing-":
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
    String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    String AP_NameString = "MRPC " + macID;

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (int i=0; i<AP_NameString.length(); i++)
        AP_NameChar[i] = AP_NameString.charAt(i);
    WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_NameChar, password);
}

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
    <input type=\"submit\" value=\"Connect\">\
    </form>";
    server.send(200, "text/html", response);
}

void handleConnect() {
    if(server.hasArg("ssid") && server.hasArg("password")) {
        Json::Object &wifi_settings = settings()["wifi"].asObject();
        wifi_settings["ssid"] = server.arg("ssid");
        wifi_settings["password"] = server.arg("password");
        save_settings();
        server.send(200, "text/html", "Successfully saved settings");
    }
    else
        server.send(500, "text/html", "Missing ssid or password");
}

bool validWifiSettings() {
    if(!settings()["wifi"].isObject()) return false;
    Json::Object &wifi_settings = settings()["wifi"].asObject();
    if(!wifi_settings["ssid"].isString() || !wifi_settings["password"].isString()) return false;
    return true;
}


void MRPC::use_transport(Transport *transport) {
    transports.append(transport);
}
void MRPC::register_service(const char* path, Service *service) {
    services[path] = service;
}

void MRPC::on_recv(Json::Object &msg) {
    if(Message::is_request(msg)) {
        Path path = Path(msg["dst"].asString());
        Service *service = get_service(path);
        if(service) {
            ServiceMethod method = service->get_method(msg["procedure"].asString());
            if(method) {
                Json::Object &response = 
                    msg["id"].isInt() ? 
                        Message::Create(msg["id"].asInt(), guid.hex, msg["src"].asString()) :
                        Message::Create(guid.hex, msg["src"].asString());
                Json::Value msg_value = msg["value"];
                bool success = true;
                Json::Value response_value = method(service, msg_value, success);
                response["result"] = response_value;
                if(success) {
                    for(int i = 0; i < transports.size(); i++) {
                        transports[i]->send(response);
                    }
                }
                delete &response;
            }
        }
    }
    else if(Message::is_response(msg)) {
        if(results.has(msg["id"].asInt())) {
            Result &result = results[msg["id"].asInt()];
            bool failure = msg["result"].type == JSON_INVALID;
            result.resolve(failure ? msg["error"] : msg["result"], failure);
        }
    }
}

Result *MRPC::rpc(const char* path, const char* procedure, Json::Value value) {
    int id = Message::id++;
    Json::Object &msg = Message::Create(id, guid.hex, path);
    msg["procedure"] = procedure;
    msg["value"] = value;
    for(int i = 0; i < transports.size(); i++)
    {
        transports[i]->send(msg);
    }
    //Don't delete the value we passed in
    msg["value"] = 0;
    delete &msg;
    return results.get_create(id);
}

Service *MRPC::get_service(Path path) {
    for (auto kvp : services)
    {
        if(strcmp(kvp.key, path.service) == 0)
            return kvp.value;
    }
    return NULL;
}