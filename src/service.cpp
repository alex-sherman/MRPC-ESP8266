#include "mrpc.h"
#include "service.h"

using namespace MRPC;

void Service::add_method(std::string str, ServiceMethod method) {
    methods[str] = method;
}

ServiceMethod Service::get_method(std::string str) {
    return methods[str];
}