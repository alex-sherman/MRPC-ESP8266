#include "service.h"
#include "mrpc.h"
#include "routing.h"

using namespace MRPC;

aJsonObject &who_has(Service* self, const aJsonObject& value, bool& success) {
    Path path = Path(value.as<const char*>());
    Service *service = self->node->get_service(path);
    if(service)
        return self->node->guid.hex;
    success = false;
}
aJsonObject &list_procedures(Service* self, const aJsonObject& value, bool& success) {
    aJsonObject& out = *aJson.createObject();
    for (auto const& service_it : self->node->services)
    {
        aJsonObject& service = aJson.createArray();
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