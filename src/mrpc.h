#ifndef _MRPC_H_
#define _MRPC_H_

#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <WiFiUDP.h>
#include "message.h"
#include "service.h"
#include "routing.h"
#include "path.h"
#include "uuid.h"
#include "amap.h"
#include "transport.h"
#include "result.h"

namespace MRPC {
    void init();
    void poll();
    Json::Object &settings();
    void save_settings();
    class Transport;
    class Service;
    class Routing;
    class UUID;

    class Node {
    public:
        Node();
        UUID guid;
        void use_transport(Transport *transport);
        void register_service(const char* path, Service *service);
        Service *get_service(Path path);
        void on_recv(Json::Object&);
        Result *rpc(const char*, const char*,Json::Value);
        AMap<Service*> services;
        void wait();
        bool poll();
    private:
        int id = 0;
        Routing *routing;
        static Node *_single;
        AList<MRPC::Transport*> transports;
        AMap<Result> results;
    };
}

#endif //_MRPC_H_