#ifndef _MRPC_H_
#define _MRPC_H_

#include <stdlib.h>
#include <sys/socket.h>
#include <stdint.h>
#include <string.h>
#include <json/json.h>
#include <thread>
#include "message.h"
#include "service.h"
#include "routing.h"
#include "path.h"
#include "uuid.h"
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
        std::vector<Callback> callbacks;
        bool completed;
        bool success;
    private:
        Json::Value value;
    };

    //Result *rpc(std::string path, std::string procedure, )

    class Node {
    public:
        static Node *Single();
        Node();
        UUID guid;
        void use_transport(Transport *transport);
        void register_service(std::string path, Service *service);
        Service *get_service(Path path);
        void on_recv(Message msg);
        Result *rpc(std::string, std::string, Json::Value);
        void wait();
        bool poll();
    private:
        int id = 0;
        Routing *routing;
        static Node *_single;
        std::vector<MRPC::Transport*> transports;
        std::map<std::string, Service*> services;
        std::map<int, Result*> results;
    };
    class Transport {
    public:
        bool poll();
        void close();
        virtual void send(Message message) = 0;
    private:
        virtual Message recv() = 0;
        std::thread recv_thread;
    };
    class SocketTransport : public Transport {
    public:
        SocketTransport();
        SocketTransport(int local_port);
        void send(Message message);
    private:
        std::map<std::string, sockaddr_storage> known_guids;
        Message recv();
        int sock;
        struct sockaddr_storage broadcast;
        struct sockaddr_storage *guid_lookup(std::string hex);
    };
    class Proxy {
    public:
        Proxy(std::string host);
        struct sockaddr_storage remote;
        int sockfd;
        int next_id;
    };
    
}

#endif //_MRPC_H_