#ifndef _MRPC_H_
#define _MRPC_H_
#define ARDUINO

#include <stdlib.h>
#include <string.h>
#include <aJSON.h>
#include <WiFiUDP.h>
#include "message.h"
#include "service.h"
#include "routing.h"
#include "path.h"
#include "uuid.h"
#include "amap.h"
#include <functional>

#ifdef ARDUINO
    #include <vector>
    #include <map>
#endif
namespace MRPC {
    class Transport;
    class Service;
    class Routing;
    class UUID;
    class Result {
        typedef std::function<void(aJsonObject &, bool)> Callback;
        
    public:
        Result() { }
        void resolve(aJsonObject &, bool success);
        void when(Callback callback);
        std::vector<Callback> callbacks;
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
        void on_recv(aJsonObject &);
        Result *rpc(const char*, const char*, aJsonObject &);
        std::map<const char*, Service*> services;
        void wait();
        bool poll();
    private:
        int id = 0;
        Routing *routing;
        static Node *_single;
        std::vector<MRPC::Transport*> transports;
        std::map<int, Result*> results;
    };
    class Transport {
    public:
        Node *node;
        bool poll();
        void close();
        virtual void send(aJsonObject &) = 0;
        virtual bool recv(char buffer[1024]) = 0;
    };
    struct UDPEndpoint {
        IPAddress ip;
        uint16_t port;
    };
    class UDPTransport : public Transport {
    public:
        UDPTransport();
        UDPTransport(int local_port);
        void send(aJsonObject &);
        bool recv(char buffer[1024]);
        struct UDPEndpoint *guid_lookup(const char *hex);
    private:
        struct UDPEndpoint broadcast;
        amap<struct UDPEndpoint> known_guids;
        WiFiUDP udp;
        uint16_t remote_port;
    };
}

#endif //_MRPC_H_