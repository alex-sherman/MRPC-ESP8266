#include "mrpc.h"
#include <iostream>

using namespace MRPC;

Json::Value echo(Service *service, Json::Value value) {
    std::cout << service->storage << "\n";
    service->storage["herp"] = value;
    std::cout << value.asString() << "\n";
    return value;
}
Json::Value echo1(Service *service, Json::Value value) {
    std::cout << value.asString() << "\n";
    return "HERP" + value.asString() + "DERP";
}

Json::Value temperature(Service *self) {
    return 75;
}

int main() {
    Transport *trans = new SocketTransport(50123);
    Node *self = Node::Single();
    Service simpleService = Service();
    simpleService.add_method("echo", &echo);
    simpleService.add_publisher("temperature", &temperature, "/LivingRoom", 1);
    self->register_service("/SimpleService", &simpleService);
    Service otherService = Service();
    otherService.add_method("echo", &echo1);
    self->register_service("/OtherService", &otherService);
    self->use_transport(trans);
    self->wait();
}