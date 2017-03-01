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
#include <ESP8266WebServer.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

namespace MRPC {
    void init(int port = 50123);
    void poll();
    Json::Object &settings();
    void save_settings();
    class UDPTransport;
    class Service;
    class Routing;
    class UUID;

    Service &create_service(const char* name, ServiceMethod method);
    Publisher &create_publisher(const char *name, PublisherMethod method, const char *path, long interval);
    void on_recv(Json::Object&, UDPEndpoint);
    Result *rpc(const char* path, Json::Value value);
    void wait();

    UUID &guid();

    class RequestCache {
    private:
        static const int N = 10;
        int offset = 0;
        int recv_ids[N];
    public:
        long recv_millis;
        bool already_responded(int id) {
            for(int i = 0; i < N; i++) {
                if(recv_ids[i] == id)
                    return true;
            }
            return false;
        }
        void add_response(int id) {
            recv_ids[offset] = id;
            offset++; offset %= N;
            recv_millis = millis();
        }
        bool stale() {
            return millis() - recv_millis > 2000;
        }
    };

    static ESP8266WebServer webserver(80);
    static uint led_indicator = LED_BUILTIN;
    static AMap<Service*> services;
    static AMap<Publisher *> publishers;
    static MRPC::UDPTransport* transport;
    static AMap<Result> results;
    static AList<char*> aliases;
    static AMap<RequestCache> responded;
}

#endif //_MRPC_H_