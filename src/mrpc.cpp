#include "mrpc.h"
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include "message.h"
#include "spi_flash.h"

using namespace MRPC;

Json::Object *eepromJSON;
ESP8266WebServer server(80);
void handleConnect();
void handleRoot();
void setupWiFiAP(const char*);
bool validWifiSettings();
int Message::id = 0;
char eeprom_buffer[1024];

Json::Value configure_service(Service *self, Json::Value &value, bool &success) {
    if(!value.isArray()) { success = false; return "Takes 2 arguments"; }
    Json::Array &args = value.asArray();
    if(!args.size() == 2) { success = false; return "Takes 2 arguments"; }
    if(!args[0].isString()) { success = false; return "First argument must be string"; }
    if(!args[1].isObject()) { success = false; return "Second argument must be object"; }
    const char*name = args[0].asString();
    if(services.has(name)) {
        delete &settings()["services"].asObject()[name].asObject();
        settings()["services"].asObject()[name] = args[1].asObject().clone();
        services[name]->configure();
    }
    return true;
}


void MRPC::init(int port) {
    EEPROM.begin(sizeof(eeprom_buffer));
    Serial.println();
    transport = new UDPTransport(port);
    Message::id = 0;
    guid = UUID();
    create_service("configure_service", &configure_service);
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

void eeprom_read(int ee, char *dst, size_t length)
{
    byte* p = (byte*)(void*)dst;
    unsigned int i;
    for (i = 0; i < length; i++) {
        *p++ = EEPROM.read(ee++);
    }
}

class Fuck {
public:
    char data[1024];
};

Json::Object &MRPC::settings() {
    if(eepromJSON == NULL) {
        eeprom_read(0, eeprom_buffer, sizeof(eeprom_buffer));
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
                        Message::Create(msg["id"].asInt(), guid.hex, msg["src"].asString()) :
                        Message::Create(guid.hex, msg["src"].asString());
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
    Json::Object &msg = Message::Create(id, guid.hex, path);
    msg["value"] = value;
    transport->send(msg, broadcast);
    //Don't delete the value we passed in
    msg["value"] = 0;
    delete &msg;
    return results.get_create(id);
}