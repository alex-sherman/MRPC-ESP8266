#include <json/json.h>
#include "service.h"
#include <iostream>
#include "mrpc.h"
#include "routing.h"

using namespace MRPC;

Json::Value *who_has(Json::Value args, Json::Value kwargs) {
    std::cout << args[0] << "\n";
}

Routing::Routing() {
    add_method("who_has", &who_has);
}