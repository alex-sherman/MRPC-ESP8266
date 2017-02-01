#include "mrpc.h"
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include "message.h"
#include "spi_flash.h"
#include "mrpc_wifi.h"

using namespace MRPC;

Json::Value doRPC(Path path, Json::Value value, bool &success);

MRPCWifi mrpcWifi;
Json::Object *eepromJSON = NULL;
UUID *_guid = NULL;
bool validWifiSettings();
int Message::id = 0;
char eeprom_buffer[1024];
void initWebserver();
void pollWebserver();

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
    if(value.isNull() || value.isInvalid()) { return service_json.clone(); }
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
    success = false;
    if(value.isObject()) {
        settings()["wifi"].free_parsed();
        settings()["wifi"] = value.asObject().clone();
        success = true;
    }
    else if(value.isArray()) {
        success = true;
        settings()["wifi"].free_parsed();
        Json::Array &value_array = value.asArray();
        Json::Object &wifi = *new Json::Object();
        settings()["wifi"] = wifi;
        if(value_array.size() >= 1)
            wifi["ssid"] = value_array[0].asString();
        if(value_array.size() >= 2)
            wifi["password"] = value_array[1].asString();
        if(value_array.size() >= 3)
            wifi["mesh_ssid"] = value_array[2].asString();
        if(value_array.size() >= 4)
            wifi["mesh_password"] = value_array[3].asString();
    }
    else if(value.isNull()) {
        success = true;
        return settings()["wifi"].asObject().clone();
    }
    if(success) {
        save_settings();
        return settings()["wifi"].asObject().clone();
    }
    return "Invalid wifi setting value";
}

Json::Value alias_service(Service *self, Json::Value &value, bool &success) {
    if(value.isNull() || value.isInvalid()) { return settings()["aliases"].asArray().clone(); }
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
    WiFi.persistent(false);
    EEPROM.begin(sizeof(eeprom_buffer));
    Serial.println();
    initWebserver();
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
    WiFi.mode(WIFI_STA);
    bool createAP = true;
    if(!settings()["wifi"].isObject()) {
        Serial.println("Couldn't find WiFi settings");
        settings()["wifi"] = *(new Json::Object());
        Json::println(settings(), Serial);
    }
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

String inputString = "";
boolean stringComplete = false;
void handleSerialRPC() {
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        inputString += inChar;
        if (inChar == '\n')
            stringComplete = true;
    }
    if(stringComplete) {
        String pathString = "*.";
        String valueString = "";
        int i = 0;
        for(; i < inputString.length() && inputString[i] != '\n'; i++) {
            if(inputString[i] == '(')
                break;
            pathString += inputString[i];
        }
        i++;
        for(; i < inputString.length() && inputString[i] != '\n'; i++) {
            if(inputString[i] == ')')
                break;
            valueString += inputString[i];
        }
        Path path = Path(pathString.c_str());
        Json::Value value = Json::parse(valueString.c_str());
        bool success = true;
        Json::Value result = doRPC(path, value, success);
        Json::println(result, Serial);
        value.free_parsed();
        result.free_parsed();

        inputString = "";
        stringComplete = false;
    }
}

void MRPC::poll() {
    handleSerialRPC();
    pollWebserver();
    mrpcWifi.poll();
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

Json::Value doRPC(Path path, Json::Value value, bool &success) {
    if(!path.is_valid) return Json::Value::invalid();
    for(auto &service : services) {
        if(path.match(service.value)) {
            return service.value->method(service.value, value, success);
        }
    }
    return Json::Value::invalid();
}

void MRPC::on_recv(Json::Object &msg, UDPEndpoint from) {
    struct UDPEndpoint forward_dst, dst;
    forward_dst = {MRPCWifi::forward_ip(from.ip), 50123};
    dst = {MRPCWifi::respond_ip(from.ip), 50123};
    transport->senddst(msg, &forward_dst);
    if(Message::is_request(msg)) {
        Path path = Path(msg["dst"].asString());
        bool success = true;
        Json::Value response_value = doRPC(path, msg["value"], success);
        if(!response_value.isInvalid()) {
            Json::Object &response =
                msg["id"].isInt() ?
                    Message::Create(msg["id"].asInt(), guid().chars, msg["src"].asString()) :
                    Message::Create(guid().chars, msg["src"].asString());
            if(success) {
                response["result"] = response_value;
            }
            else {
                response["error"] = response_value;
            }
            transport->senddst(response, &dst);
            delete &response;
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

Result *MRPC::rpc(const char* path, Json::Value value) {
    int id = Message::id++;
    Json::Object &msg = Message::Create(id, guid().chars, path);
    msg["value"] = value;
    transport->send(msg);
    //Don't delete the value we passed in
    msg["value"] = 0;
    delete &msg;
    return results.get_create(id);
}