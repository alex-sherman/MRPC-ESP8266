#ifndef _JRPC_H_
#define _JRPC_H_

#include <stdlib.h>
#include <sys/socket.h>
#include <stdint.h>
#include <string.h>
#include <json/json.h>
#include <thread>

namespace MRPC {

    class Transport;
    class Service;
    class Message;

    class Node {
    public:
        static Node *Single();
        Node();
        std::string guid;
        void use_transport(Transport *transport);
        void wait();
    private:
        static Node *_single;
        std::vector<MRPC::Transport*> transports;
        std::map<std::string, Service> services;
    };
    class Transport {
    public:
        void begin();
        virtual void send(Message *message) = 0;
        virtual Message *recv() = 0;
        void close();
    private:
        std::thread recv_thread;
    };
    class SocketTransport : public Transport {
    public:
        SocketTransport();
        SocketTransport(int local_port);
        void send(Message *message);
        Message *recv();
    private:
        int sock;
    };
    class Proxy {
    public:
        Proxy(std::string host);
        struct sockaddr_storage remote;
        int sockfd;
        int next_id;
    };
    class Service {

    };
    
}

#endif //_JRPC_H_