#include "service.h"
#include "mrpc.h"
#include "routing.h"

using namespace MRPC;

Json::Value who_has(Service* self, Json::Value& value, bool& success) {
    Path path = Path(value.asString());
    Service *service = self->node->get_service(path);
    if(service)
        return self->node->guid.hex;
    success = false;
}
Json::Value list_procedures(Service* self, Json::Value& value, bool& success) {
    Json::Object out;
    for (auto const& service_it : self->node->services)
    {
        Json::Array service;
        for (auto const& method : service_it.value->methods)
        {
            service.append(method.key);
        }
        out[service_it.key] = service;
    }
    return out;
}

Routing::Routing() {
    add_method("who_has", &who_has);
    add_method("list_procedures", &list_procedures);
}