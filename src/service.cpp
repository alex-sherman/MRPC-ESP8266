#include "mrpc.h"
#include "service.h"
#include <iostream>

using namespace MRPC;

Json::Value get_publications(Service *self, Json::Value value) {
    Json::Value out = Json::Value(Json::ValueType::objectValue);
    for (auto const& it : self->publishers)
    {
        Json::Value publisher = Json::Value(Json::ValueType::objectValue);
        publisher["interval"] = it.second->interval;
        publisher["procedure"] = it.second->procedure;
        publisher["path"] = it.second->path;
        out[it.first] = publisher;

    }
    return out;
}
Json::Value set_publication(Service *self, Json::Value value) {
    return self->storage["publications"][value["name"].asString()] = value["value"];
}

Service::Service() {
    add_method("get_publications", get_publications);
    add_method("set_publication", set_publication);
}

void Service::add_method(std::string str, ServiceMethod method) {
    methods[str] = method;
}

ServiceMethod Service::get_method(std::string str) {
    return methods[str];
}

void Service::add_publisher(std::string name, PublisherMethod method, std::string path, int interval) {
    publishers[name] = new Publisher(method, path, name, interval);
}

void Service::update(uint64_t time) {
    for (auto const& it : publishers)
    {
        Publisher *publisher = it.second;
        if(publisher->interval == 0) continue;
        if(time - publisher->last_called > publisher->interval) {
            publisher->last_called = time;
            Node::Single()->rpc(publisher->path, publisher->procedure, publisher->method());
            std::cout << publisher->method() << "\n";
        }
    }
}

Publisher::Publisher(PublisherMethod method, std::string path, std::string procedure, uint interval) {
    this->method = method;
    this->path = path;
    this->procedure = procedure;
    this->interval = interval;
}