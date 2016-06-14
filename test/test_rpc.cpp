#include "mrpc.h"
#include <iostream>

using namespace MRPC;

Json::Value echo(Json::Value args, Json::Value kwargs) {
    std::cout << args[0].asString() << "\n";
    return args[0];
}
Json::Value echo1(Json::Value args, Json::Value kwargs) {
    std::cout << args[0].asString() << "\n";
    return "HERP" + args[0].asString() + "DERP";
}

int main() {
    Transport *trans = new SocketTransport(50123);
    Node *self = Node::Single();
    Service simpleService = Service();
    simpleService.add_method("echo", &echo);
    self->register_service("/SimpleService", &simpleService);
    Service otherService = Service();
    otherService.add_method("echo", &echo1);
    self->register_service("/OtherService", &otherService);
    self->use_transport(trans);
    self->wait();
}