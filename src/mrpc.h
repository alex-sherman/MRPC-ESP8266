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

namespace MRPC {

    class Transport;
    class Service;
    class Routing;

    class Node {
    public:
        static Node *Single();
        Node();
        std::string guid;
        void use_transport(Transport *transport);
        void register_service(std::string path, Service *service);
        void on_recv(Message *msg);
        void wait();
        bool poll();
    private:
        Routing *routing;
        static Node *_single;
        std::vector<MRPC::Transport*> transports;
        std::map<std::string, Service*> services;
    };
    class Transport {
    public:
        bool poll();
        void close();
    private:
        virtual void send(Message *message) = 0;
        virtual int recv(char *buffer, size_t size) = 0;
        std::thread recv_thread;
    };
    class SocketTransport : public Transport {
    public:
        SocketTransport();
        SocketTransport(int local_port);
    private:
        void send(Message *message);
        int recv(char *buffer, size_t size);
        int sock;
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