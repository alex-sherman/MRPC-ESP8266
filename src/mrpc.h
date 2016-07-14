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
#include <functional>

namespace MRPC {
    class Transport;
    class Service;
    class Routing;
    class UUID;
    class Result {
        typedef std::function<void(Json::Value, bool)> Callback;
        
    public:
        Result() { }
        void resolve(Json::Value, bool success);
        void when(Callback callback);
        AList<Callback> callbacks;
        bool completed;
        bool success;
    };

    //Result *rpc(std::string path, std::string procedure, )

    class Node {
    public:
        Node();
        UUID guid;
        void use_transport(Transport *transport);
        void register_service(const char* path, Service *service);
        Service *get_service(Path path);
        void on_recv(Json::Object);
        Result *rpc(const char*, const char*,Json::Value);
        AMap<Service*> services;
        void wait();
        bool poll();
    private:
        int id = 0;
        Routing *routing;
        static Node *_single;
        AList<MRPC::Transport*> transports;
        AMap<Result*> results;
    };
    class Transport {
    public:
        Node *node;
        bool poll();
        void close();
        virtual void send(Json::Object) = 0;
        virtual Json::Object recv() = 0;
    };
    struct UDPEndpoint {
        IPAddress ip;
        uint16_t port;
    };
    class UDPTransport : public Transport {
    public:
        UDPTransport();
        UDPTransport(int local_port);
        void send(Json::Object);
        Json::Object recv();
        struct UDPEndpoint *guid_lookup(const char *hex);
    private:
        struct UDPEndpoint broadcast;
        AMap<struct UDPEndpoint> known_guids;
        WiFiUDP udp;
        uint16_t remote_port;
    };
}

#endif //_MRPC_H_