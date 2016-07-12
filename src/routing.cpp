#include "service.h"
#include "mrpc.h"
#include "routing.h"

using namespace MRPC;

JsonVariant who_has(Service* self, const JsonVariant& value, StaticJsonBuffer<2048>* messageBuffer, bool& success) {
    Path path = Path(value.as<const char*>());
    Service *service = self->node->get_service(path);
    if(service)
        return self->node->guid.hex;
    success = false;
}
JsonVariant list_procedures(Service* self, const JsonVariant& value, StaticJsonBuffer<2048>* messageBuffer, bool& success) {
    JsonObject& out = messageBuffer->createObject();
    for (auto const& service_it : self->node->services)
    {
        JsonArray& service = messageBuffer->createArray();
        for (auto const& method : service_it.second->methods)
        {
            service.add(method.first);
        }
        out[service_it.first] = service;
    }
    return out;
}

Routing::Routing() {
    add_method("who_has", &who_has);
    add_method("list_procedures", &list_procedures);
}