#include "mrpc.h"
#include "service.h"

using namespace MRPC;

Json::Value get_publications(Service *self, Json::Value args, Json::Value kwargs) {
    Json::Value out = Json::Value(Json::ValueType::arrayValue);
    Json::Value::Members members = self->storage["publications"].getMemberNames();
    for(int i = 0; i < members.size(); i++) {
        out.append(members[i]);
    }
    return out;
}
Json::Value set_publication(Service *self, Json::Value args, Json::Value kwargs) {
    return self->storage["publications"][args[0].asString()] = args[1];
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

void Service::add_publisher(std::string name, PublisherMethod method, std::string path) {
    publishers[name] = Publisher(method, path, name);
}

Publisher::Publisher(PublisherMethod method, std::string path, std::string procedure, int interval) {
    this->method = method;
    this->path = path;
    this->procedure = procedure;
    this->interval = interval;
}