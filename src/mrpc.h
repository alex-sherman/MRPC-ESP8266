#pragma once

#ifndef _MRPC_H_
#define _MRPC_H_

#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <WiFiUdp.h>
#include "message.h"
#include "service.h"
#include "path.h"
#include "uuid.h"
#include "amap.h"
#include "transport.h"
#include "result.h"

namespace MRPC {
    void init(int port = 50123);
    void respond(const char *name);
    void poll();
    Json::Object &settings();
    void save_settings();
    class Transport;
    class Service;
    class Routing;
    class UUID;

    static UUID guid;
    Service &create_service(const char* name, ServiceMethod method);
    Publisher &create_publisher(const char *name, PublisherMethod method, const char *path, long interval);
    void on_recv(Json::Object&);
    Result *rpc(const char* path, Json::Value value, bool broadcast = false);
    void wait();

    static AMap<Service*> services;
    static AMap<Publisher *> publishers;
    static MRPC::Transport* transport;
    static AMap<Result> results;

    static AList<char*> aliases;
}

#endif //_MRPC_H_