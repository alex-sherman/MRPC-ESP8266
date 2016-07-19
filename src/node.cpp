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
}

void Node::on_recv(Json::Object &msg) {
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

Result *Node::rpc(const char* path, const char* procedure, Json::Value value) {
    int id = this->id++;
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

Service *Node::get_service(Path path) {
    for (auto kvp : services)
    {
        if(strcmp(kvp.key, path.service) == 0)
            return kvp.value;
    }
    return NULL;
}

bool Node::poll() {
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
    return output;
}

void Node::wait() {
    while(true) {
        poll();
        delay(5);
    }
}