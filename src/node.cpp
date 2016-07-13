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

void Node::on_recv(aJsonObject &msg) {
    if(Message::is_request(msg)) {
        Serial.println("Received a request");
        Path path = Path(msg.get("dst").valuestring);
        Service *service = get_service(path);
        if(service) {
            Serial.print("Finding method: ");
            msg.get("procedure").printTo(Serial);
            ServiceMethod method = service->get_method(msg.get("procedure").valuestring);
            if(method) {
                Serial.println("Have method");
                aJsonObject & response = 
                    msg.get("id").type == aJson_Int ? 
                        Message::Create(msg.get("id").valueint, guid.hex, msg.get("src").valuestring) :
                        Message::Create(guid.hex, msg.get("src").valuestring);
                aJsonObject & msg_value = msg.get("value");
                bool success = true;
                aJsonObject *response_value = (aJsonObject *)malloc(sizeof(aJsonObject));
                *response_value = method(service, msg_value, success);
                response.set("result", *response_value);
                if(success) {
                    for(int i = 0; i < transports.size(); i++) {
                        transports[i]->send(response);
                    }
                }
            }
        }
    }
    else if(Message::is_response(msg)) {
        Serial.println("Received a response");
        Result *result = results[msg["id"].valueint];
        if(result) {
            bool failure = msg["result"].empty;
            result->resolve(failure ? msg["error"] : msg["result"], failure);
        }
    }
}

Result *Node::rpc(const char* path, const char* procedure, aJsonObject &value) {
    Serial.println("Node::RPC()");
    int id = this->id++;
    aJsonObject &msg = Message::Create(id, guid.hex, path);
    msg.set("procedure", procedure);
    msg.set("value", value);
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