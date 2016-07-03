#include <json/json.h>
#include "service.h"
#include <iostream>
#include "mrpc.h"
#include "routing.h"
#include "exception.h"

using namespace MRPC;

Json::Value who_has(Service *self, Json::Value value) {
    Path path = Path(value.asString());
    Service *service = Node::Single()->get_service(path);
    if(service)
        return Node::Single()->guid.hex;
    throw NoReturn("No method " + path.service + " could be found");
}
Json::Value list_procedures(Service *self, Json::Value value) {
    Json::Value out = Json::Value();
    for (auto const& service_it : Node::Single()->services)
    {
        Json::Value service = Json::Value(Json::ValueType::arrayValue);
        for (auto const& method : service_it.second->methods)
        {
            service.append(method.first);
        }
        out[service_it.first] = service;
    }
    return out;
}

Routing::Routing() {
    add_method("who_has", &who_has);
    add_method("list_procedures", &list_procedures);
}