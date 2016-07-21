#include "mrpc.h"
#include "service.h"
#include <Arduino.h>

using namespace MRPC;

Json::Value get_publications(Service* self, Json::Value& value, bool& success) {
    Json::Object &out = *(new Json::Object());
    for (auto const& it : self->publishers)
    {
        Json::Object &publisher = *(new Json::Object());
        publisher["interval"] = it.value->interval;
        publisher["procedure"] = it.value->procedure;
        out[it.key] = publisher;
    }
    return out;
}
Json::Value set_publication(Service* self, Json::Value& value, bool& success) {
    //return self->storage["publications"][value["name"].asString()] = value["value"];
}

Service::Service(const char *name) {
    Path p = Path(name);
    this->path = prefix.concat(p);
    strncpy(this->name, name, sizeof(this->name));
    add_method("get_publications", get_publications);
    add_method("set_publication", set_publication);
}

void Service::add_method(const char* str, ServiceMethod method) {
    methods[str] = method;
}

ServiceMethod Service::get_method(const char* str) {
    for (auto const& it : methods) {
        if(strcmp(str, it.key) == 0)
            return it.value;
    }
    return NULL;
}

void Service::add_publisher(const char* name, PublisherMethod method, int interval) {
    publishers[name] = new Publisher(method, name, interval);
}

void Service::update(uint64_t time) {
    for (auto const& it : publishers)
    {
        Publisher *publisher = it.value;
        if(publisher->interval == 0) continue;
        if(time - publisher->last_called > publisher->interval) {
            publisher->last_called = time;
            Json::Value result = publisher->method(this);
            rpc(this->path.path, publisher->procedure, result);
            result.free_parsed();
        }
    }
}

Publisher::Publisher(PublisherMethod method, const char* procedure, uint interval) {
    this->method = method;
    this->procedure = procedure;
    this->interval = interval;
}