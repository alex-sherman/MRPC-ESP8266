#include "mrpc.h"
#include "service.h"
#include <Arduino.h>

using namespace MRPC;

JsonVariant get_publications(Service* self, const JsonVariant& value, StaticJsonBuffer<2048>* messageBuffer, bool& success) {
    JsonObject& out = messageBuffer->createObject();
    for (auto const& it : self->publishers)
    {
        JsonObject& publisher = out.createNestedObject(it.first);
        publisher["interval"] = it.second->interval;
        publisher["procedure"] = it.second->procedure;
        publisher["path"] = it.second->path;

    }
    return out;
}
JsonVariant set_publication(Service* self, const JsonVariant& value, StaticJsonBuffer<2048>* messageBuffer, bool& success) {
    //return self->storage["publications"][value["name"].asString()] = value["value"];
}

Service::Service() {
    publishers = std::map<const char*, Publisher*>();
    methods = std::map<const char*, ServiceMethod>();
    add_method("get_publications", get_publications);
    add_method("set_publication", set_publication);
}

void Service::add_method(const char* str, ServiceMethod method) {
    methods[str] = method;
}

ServiceMethod Service::get_method(const char* str) {
    for (auto const& it : methods) {
        Serial.print(str);
        Serial.print(strcmp(str, it.first));
        Serial.println(it.first);
        if(strcmp(str, it.first) == 0)
            return it.second;
    }
    return NULL;
}

void Service::add_publisher(const char* name, PublisherMethod method, const char* path, int interval) {
    publishers[name] = new Publisher(method, path, name, interval);
}

void Service::update(uint64_t time) {
    StaticJsonBuffer<2048> messageBuffer;
    /*for (auto const& it : publishers)
    {
        Publisher *publisher = it.second;
        if(publisher->interval == 0) continue;
        if(time - publisher->last_called > publisher->interval) {
            publisher->last_called = time;
            node->rpc(publisher->path, publisher->procedure, publisher->method(this, &messageBuffer), &messageBuffer);
        }
    }*/
}

Publisher::Publisher(PublisherMethod method, const char* path, const char* procedure, uint interval) {
    this->method = method;
    this->path = path;
    this->procedure = procedure;
    this->interval = interval;
}