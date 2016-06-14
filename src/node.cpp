#include "jrpc.h"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <thread>

using namespace MRPC;
Node *Node::_single = new Node();
Node::Node() {
    guid = "HERP";
    transports = std::vector<MRPC::Transport*>();
    services = std::map<std::string, Service>();
}
Node *Node::Single() {
    return Node::_single;
}

void Node::use_transport(Transport *transport) {
    transport->begin();
    transports.push_back(transport);
}

void Node::wait() {
    while(true) {
        usleep(1000);
    }
}