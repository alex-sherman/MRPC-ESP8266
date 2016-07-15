#include "mrpc.h"
#include "service.h"
#include <stdlib.h>
#include "path.h"
#include "uuid.h"
#include "Arduino.h"

using namespace MRPC;

Node::Node() {
    guid = UUID();
    routing = new Routing();
    register_service("/Routing", routing);
}

void Node::use_transport(Transport *transport) {
    transport->node = this;
    transports.append(transport);
}
void Node::register_service(const char* path, Service *service) {
    service->node = this;
    services[path] = service;
    Serial.print("Added service:");
    Serial.println(path);
}

void Node::on_recv(Json::Object msg) {
    if(Message::is_request(msg)) {
        Serial.println("Received a request");
        Path path = Path(msg["dst"].asString());
        Service *service = get_service(path);
        if(service) {
            Serial.print("Finding method: ");
            Serial.println(msg["procedure"].asString());
            ServiceMethod method = service->get_method(msg["procedure"].asString());
            if(method) {
                Serial.println("Have method");
                Json::Object response = 
                    msg["id"].isInt() ? 
                        Message::Create(msg["id"].asInt(), guid.hex, msg["src"].asString()) :
                        Message::Create(guid.hex, msg["src"].asString());
                Json::Value msg_value = msg["value"];
                bool success = true;
                Json::Value response_value = method(service, msg_value, success);
                Serial.println("Result in node");
                response["result"] = response_value;
                if(success) {
                    for(int i = 0; i < transports.size(); i++) {
                        transports[i]->send(response);
                    }
                }
                Json::println(response_value, Serial);
            }
        }
    }
    else if(Message::is_response(msg)) {
        Serial.println("Received a response");
        Result *result = results[msg["id"].valueint];
        if(result) {
            bool failure = msg["result"].type == JSON_INVALID;
            result->resolve(failure ? msg["error"] : msg["result"], failure);
        }
    }
}

Result *Node::rpc(const char* path, const char* procedure, Json::Value value) {
    Serial.println("Node::RPC()");
    int id = this->id++;
    Json::Object msg = Message::Create(id, guid.hex, path);
    msg["procedure"] = procedure;
    msg["value"] = value;
    Result *result = new Result();
    results[id] = result;
    for(int i = 0; i < transports.size(); i++)
    {
        transports[i]->send(msg);
    }
    return result;
}

Service *Node::get_service(Path path) {
    Serial.print("Looking up service: ");
    Serial.println(path.service);
    for (int i = 0; i < services.elements.size(); i++)
    {
        auto kvp = services.elements[i];
        if(strcmp(kvp.key, path.service) == 0)
            return kvp.value;
    }
    return NULL;
}

bool Node::poll() {
    bool output = false;
    for(int i = 0; i < transports.size(); i++) {
        output |= transports[i]->poll();
    }
    for (int i = 0; i < services.elements.size(); i++)
    {
        services.elements[i].value->update(millis());
    }
    return output;
}

void Node::wait() {
    while(true) {
        poll();
        delay(5);
    }
}