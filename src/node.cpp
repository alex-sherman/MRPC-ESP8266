#include "mrpc.h"
#include "service.h"
#include <stdlib.h>
#include "path.h"
#include "uuid.h"
#include "Arduino.h"

using namespace MRPC;

Node::Node() {
    guid = UUID();
    transports = std::vector<MRPC::Transport*>();
    services = std::map<const char*, Service*>();
    routing = new Routing();
    register_service("/Routing", routing);
}

void Node::use_transport(Transport *transport) {
    transport->node = this;
    transports.push_back(transport);
}
void Node::register_service(const char* path, Service *service) {
    service->node = this;
    services[path] = service;
    Serial.print("Added service:");
    Serial.println(path);
}

void Node::on_recv(JsonObject& msg, StaticJsonBuffer<2048>* jsonBuffer) {
    if(Message::is_request(msg)) {
        Serial.println("Received a request");
        Path path = Path(msg["dst"].asString());
        Service *service = get_service(path);
        if(service) {
            Serial.print("Finding method: ");
            Serial.println(msg.get<const char*>("procedure"));
            ServiceMethod method = service->get_method(msg["procedure"].as<const char*>());
            if(method) {
                Serial.println("Have method");
                JsonObject& response = 
                    msg.get("id").is<int>() ? 
                        Message::Create(msg.get("id").as<int>(), guid.hex, msg.get("src").as<const char*>(), jsonBuffer) :
                        Message::Create(guid.hex, msg.get("src").as<const char*>(), jsonBuffer);
                const JsonVariant& msg_value = msg["value"];
                bool success = true;
                JsonVariant response_value = method(service, msg_value, jsonBuffer, success);
                response["result"] = response_value;
                if(success) {
                    for(int i = 0; i < transports.size(); i++) {
                        transports[i]->send(response, jsonBuffer);
                    }
                }
            }
        }
    }
    else if(Message::is_response(msg)) {
        Serial.println("Received a response");
        Result *result = results[msg["id"].as<int>()];
        if(result) {
            bool failure = !msg["result"];
            result->resolve(failure ? msg["error"] : msg["result"], failure);
        }
    }
}

Result *Node::rpc(const char* path, const char* procedure, JsonObject& value, StaticJsonBuffer<2048>* jsonBuffer) {
    Serial.println("Node::RPC()");
    int id = this->id++;
    JsonObject& msg = Message::Create(id, guid.hex, path, jsonBuffer);
    msg["procedure"] = procedure;
    msg["value"] = value;
    Result *result = new Result();
    results[id] = result;
    for(int i = 0; i < transports.size(); i++)
    {
        transports[i]->send(msg, jsonBuffer);
    }
    return result;
}

Service *Node::get_service(Path path) {
    Serial.print("Looking up service: ");
    Serial.println(path.service);
    for (auto const& it : services) {
        if(strcmp(it.first, path.service) == 0)
            return it.second;
    }
    return NULL;
}

bool Node::poll() {
    bool output = false;
    for(int i = 0; i < transports.size(); i++) {
        output |= transports[i]->poll();
    }
    for (auto const& it : services)
    {
        it.second->update(millis());
    }
    return output;
}

void Node::wait() {
    while(true) {
        poll();
        delay(5);
    }
}