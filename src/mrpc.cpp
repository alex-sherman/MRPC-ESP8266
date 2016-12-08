#include "mrpc.h"
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include "message.h"
#include "spi_flash.h"
#include "mrpc_wifi.h"

using namespace MRPC;

MRPCWifi mrpcWifi;
Json::Object *eepromJSON = NULL;
UUID *_guid = NULL;
ESP8266WebServer server(80);
void handleConnect();
void handleReset();
void handleRoot();
void setupWiFiAP(const char*);
bool validWifiSettings();
int Message::id = 0;
char eeprom_buffer[1024];

char*configure_service_error = "Argument must be either null, string, or [string, object]";

Json::Value reset_service(Service *self, Json::Value &value, bool &success) {
    if(value.isBool() && value.asBool()) {
        eepromJSON = new Json::Object();
        save_settings();
        ESP.restart();
        return true;
    }
    return false;
}
Json::Value configure_service(Service *self, Json::Value &value, bool &success) {
    Json::Object &service_json = settings()["services"].asObject();
    if(value.isNull()) { return service_json.clone(); }
    if(value.isString()) {
        if(service_json[value.asString()].isObject()) {
            return service_json[value.asString()].asObject().clone();
        }
        else { success = false; return "Uknown service"; }
    }
    if(!value.isArray()) { success = false; return configure_service_error; }
    Json::Array &args = value.asArray();
    if(!args.size() == 2 || !args[0].isString() || !args[1].isObject()) {
        success = false;
        return configure_service_error;
    }
    const char*name = args[0].asString();
    if(services.has(name)) {
        delete &settings()["services"].asObject()[name].asObject();
        settings()["services"].asObject()[name] = args[1].asObject().clone();
        save_settings();
        services[name]->configure();
    }
    return true;
}

Json::Value uuid_service(Service *self, Json::Value &value, bool &success) {
    return guid().chars;
}
Json::Value wifi_settings_service(Service *self, Json::Value &value, bool &success) {
    settings()["wifi"] = value;
    save_settings();
    ESP.restart();
    return true;
}

Json::Value alias_service(Service *self, Json::Value &value, bool &success) {
    if(value.isNull()) { return settings()["aliases"].asArray().clone(); }
    else if(value.isString()) {
        bool add = true;
        for(auto &alias : settings()["aliases"].asArray()) {
            if(strcmp(alias.asString(), value.asString()) == 0) {
                add = false;
                break;
            }
        }
        if(add) {
            settings()["aliases"].asArray().append(value.asString());
            save_settings();
        }
    }
    else if(value.isArray()) { settings()["aliases"].free_parsed(); settings()["aliases"] = value.asArray().clone(); save_settings(); }
    else { success = false; return "Argument must be either null, string, or [string,]"; }
    return true;
}

UUID &MRPC::guid() {
    if(_guid == NULL) {
        if(!settings()["uuid"].isString() || !UUID::is(settings()["uuid"].asString())) {
            settings()["uuid"] = UUID().chars;
            save_settings();
        }
        _guid = new UUID(settings()["uuid"].asString());
    }
    return *_guid;
}

void MRPC::init(int port) {
    EEPROM.begin(sizeof(eeprom_buffer));
    Serial.println();
    transport = new UDPTransport(port);
    Message::id = 0;
    if(!settings()["aliases"].isArray())
        settings()["aliases"] = new Json::Array();
    Serial.print("Device UUID: ");
    Serial.println(guid().chars);
    create_service("configure_service", &configure_service);
    create_service("uuid", &uuid_service);
    create_service("alias", &alias_service);
    create_service("reset", &reset_service);
    create_service("wifi", &wifi_settings_service);
    WiFi.mode(WIFI_AP_STA);
    bool createAP = true;
    if(!settings()["wifi"].isObject()) {
        Serial.println("Couldn't find WiFi settings");
        settings()["wifi"] = *(new Json::Object());
        Json::println(settings(), Serial);
    }

    Json::Object &wifi_settings = settings()["wifi"].asObject();
    mrpcWifi.connect(wifi_settings);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Starting server");
    server.on("/", HTTP_GET, handleRoot);
    server.on("/connect", HTTP_GET, handleConnect);
    server.on("/reset", HTTP_GET, handleReset);
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

    transport->poll();
    unsigned long time = millis();
    for (auto const& it : publishers)
    {
        Publisher *publisher = it.value;
        if(publisher->interval == 0) continue;
        if(time - publisher->last_called > publisher->interval) {
            publisher->last_called = time;
            Json::Value result = publisher->method();
            rpc(publisher->path, result);
            result.free_parsed();
        }
    }
}

void eeprom_write(int ee, char *src, size_t length)
{
    const byte* p = (const byte*)(const void*)src;
    unsigned int i;
    for (i = 0; i < length; i++) {
        EEPROM.write(ee++, *p++);
    }
}

void eeprom_read_string(int ee, char *dst, size_t length)
{
    byte* p = (byte*)(void*)dst;
    unsigned int i;
    for (i = 0; i < length; i++) {
        byte r = EEPROM.read(ee++);
        *p++ = r;
        if(r == 0)
            break;
    }
    for(; i < length; i++)
        *p++ = 0;
}

Json::Object &MRPC::settings() {
    if(eepromJSON == NULL) {
        eeprom_read_string(0, eeprom_buffer, sizeof(eeprom_buffer));
        Json::Value output = Json::parse(eeprom_buffer);
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
    Json::dump(settings(), eeprom_buffer, sizeof(eeprom_buffer));
    Serial.print("Saving to eeprom: ");
    Serial.println(eeprom_buffer);
    EEPROM.put(0, eeprom_buffer);
    EEPROM.commit();
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

void handleReset() {
    eepromJSON = new Json::Object();
    save_settings();
    ESP.restart();
}
void handleConnect() {
    if(server.hasArg("ssid") && server.hasArg("password")) {
        Json::Object &wifi_settings = settings()["wifi"].asObject();
        wifi_settings["ssid"] = server.arg("ssid");
        wifi_settings["password"] = server.arg("password");
        save_settings();
        server.send(200, "text/html", "Successfully saved settings");
        ESP.restart();
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

Service &MRPC::create_service(const char* name, ServiceMethod method) {
    Service *service = new Service(name, method);
    services[name] = service;
    return *service;
}

Publisher &MRPC::create_publisher(const char* name, PublisherMethod method, const char *path, long interval) {
    Publisher *publisher = new Publisher(method, path, interval);
    publishers[name] = publisher;
    return *publisher;
}

void MRPC::on_recv(Json::Object &msg) {
    if(Message::is_request(msg)) {
        Path path = Path(msg["dst"].asString());
        if(!path.is_valid) return;
        for(auto &service : services) {
            if(path.match(service.value)) {
                Json::Object &response = 
                    msg["id"].isInt() ? 
                        Message::Create(msg["id"].asInt(), guid().chars, msg["src"].asString()) :
                        Message::Create(guid().chars, msg["src"].asString());
                Json::Value msg_value = msg["value"];
                bool success = true;
                Json::Value response_value = service.value->method(service.value, msg_value, success);
                if(success) {
                    response["result"] = response_value;
                }
                else {
                    response["error"] = response_value;
                }
                transport->send(response, false);
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

Result *MRPC::rpc(const char* path, Json::Value value, bool broadcast) {
    int id = Message::id++;
    Json::Object &msg = Message::Create(id, guid().chars, path);
    msg["value"] = value;
    transport->send(msg, broadcast);
    //Don't delete the value we passed in
    msg["value"] = 0;
    delete &msg;
    return results.get_create(id);
}