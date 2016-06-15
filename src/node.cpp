#include "mrpc.h"
#include "service.h"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <thread>
#include "path.h"
#include "exception.h"
#include <exception>
#include "uuid.h"
#include <time.h>

using namespace MRPC;
Node *Node::_single = new Node();
Node::Node() {
    guid = UUID();
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

void Node::on_recv(Message msg) {
    if(msg.is_request()) {
        Path path = Path(msg["dst"].asString());
        Service *service = get_service(path);
        if(service) {
            ServiceMethod method = service->get_method(msg["procedure"].asString());
            if(method) {
                Message response;
                if(!msg["id"])
                    response = Message::Create(guid.hex, msg["src"].asString());
                else
                    response = Message::Create(msg["id"].asInt(), guid.hex, msg["src"].asString());
                try {
                    Json::Value result = method(service, msg["args"], msg["kwargs"]);
                    response["result"] = result;
                }
                catch(NoReturn &e) {
                    return;
                }
                catch(std::exception &e) {
                    response["error"] = e.what();
                    std::cout << e.what() << "\n";
                }
                for(int i = 0; i < transports.size(); i++) {
                    transports[i]->send(response);
                }
            }
        }
    }
    else if(msg.is_response()) {
        Result *result = results[msg["id"].asInt()];
        if(result) {
            bool failure = !msg["result"];
            result->resolve(failure ? msg["error"] : msg["result"], failure);
        }
    }
}

Result *Node::rpc(std::string path, std::string procedure, Json::Value value) {
    int id = this->id++;
    Message msg = Message::Create(id, guid.hex, path);
    msg["procedure"] = procedure;
    msg["value"] = value;
    Result *result = new Result();
    results[id] = result;
    for(int i = 0; i < transports.size(); i++)
    {
        transports[i]->send(msg);
    }
    return result;
}

Service *Node::get_service(Path path) {
    std::map<std::string, Service*>::iterator it;
    it = services.find(path.service);
    if(it != services.end())
        return it->second;
    return NULL;
}

bool Node::poll() {
    bool output = false;
    uint64_t time = std::time(nullptr);
    for(int i = 0; i < transports.size(); i++) {
        output |= transports[i]->poll();
    }
    for (auto const& it : services)
    {
        it.second->update(time);
    }
    return output;
}

void Node::wait() {
    while(true) {
        poll();
        usleep(1000);
    }
}