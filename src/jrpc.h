#ifndef _JRPC_H_
#define _JRPC_H_

#include <stdlib.h>
#include <sys/socket.h>
#include <stdint.h>
#include <string.h>
#include <json/json.h>
#include "uthash.h"

namespace MRPC {

    class Transport;
    class Service;

    class Node {
    public:
        Node();
        std::string guid;
    private:
        std::vector<MRPC::Transport> transports;
        std::map<std::string, Service> services;
    };
    class Transport {

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
    Node LocalNode;
    
}

#endif //_JRPC_H_