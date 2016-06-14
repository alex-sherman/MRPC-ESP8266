#include "mrpc.h"
#include "service.h"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <thread>
#include "path.h"

using namespace MRPC;
Node *Node::_single = new Node();
Node::Node() {
    guid = "HERP";
    transports = std::vector<MRPC::Transport*>();
    services = std::map<std::string, Service*>();
    routing = new Routing();
    register_service("/Routing", routing);
}
Node *Node::Single() {
    return Node::_single;
}

void Node::use_transport(Transport *transport) {
    transports.push_back(transport);
}
void Node::register_service(std::string path, Service *service) {
    services[path] = service;
}

void Node::on_recv(Message *msg) {
    if(msg->is_request()) {
        Path path = Path((*msg)["dst"].asString());
        Service *service = services[path.service];
        if(service) {
            ServiceMethod method = service->get_method((*msg)["procedure"].asString());
            if(method)
                method((*msg)["args"], (*msg)["kwargs"]);
        }
    }
}

bool Node::poll() {
    bool output = false;
    for(int i = 0; i < transports.size(); i++) {
        output |= transports[i]->poll();
    }
    return output;
}

void Node::wait() {
    while(true) {
        poll();
        usleep(1000);
    }
}