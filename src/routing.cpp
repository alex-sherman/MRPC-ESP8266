#include <json/json.h>
#include "service.h"
#include <iostream>
#include "mrpc.h"
#include "routing.h"
#include "exception.h"

using namespace MRPC;

Json::Value who_has(Service *self, Json::Value args, Json::Value kwargs) {
    Path path = Path(args[0].asString());
    Service *service = Node::Single()->get_service(path);
    if(service)
        return Node::Single()->guid.hex;
    throw NoReturn("No method " + path.service + " could be found");
}

Routing::Routing() {
    add_method("who_has", &who_has);
}