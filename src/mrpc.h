#pragma once

#ifndef _MRPC_H_
#define _MRPC_H_

#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <WiFiUdp.h>
#include "message.h"
#include "service.h"
#include "routing.h"
#include "path.h"
#include "uuid.h"
#include "amap.h"
#include "transport.h"
#include "result.h"

namespace MRPC {
    void init(int port = 50123, const char*prefix = "/");
    void poll();
    Json::Object &settings();
    void save_settings();
    class Transport;
    class Service;
    class Routing;
    class UUID;

    static UUID guid;
    void use_transport(Transport *transport);
    Service &create_service(const char* name);
    void register_service(Service *service);
    Service *get_service(Path path);
    void on_recv(Json::Object&);
    Result *rpc(const char* path, const char* procedure,Json::Value value);
    static AMap<Service*> services;
    void wait();

    static Path prefix = Path("/");
    static Routing *routing;
    static AList<MRPC::Transport*> transports;
    static AMap<Result> results;
}

#endif //_MRPC_H_