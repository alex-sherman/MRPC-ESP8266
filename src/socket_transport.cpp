#include "mrpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include "exception.h"

using namespace MRPC;

SocketTransport::SocketTransport()
: SocketTransport(0) {
}

SocketTransport::SocketTransport(int local_port) {
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    ((struct sockaddr_in*)&broadcast)->sin_family = AF_INET;
    ((struct sockaddr_in*)&broadcast)->sin_addr.s_addr = 0xFFFFFFFF;
    ((struct sockaddr_in*)&broadcast)->sin_port = htons(local_port);

    struct sockaddr_in myaddr;
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(local_port);

    if (bind(sock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
    }

    int broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, 
          sizeof(broadcastPermission)) < 0)
        perror("setsockopt() failed");
}

void sendmsg(int sock, Message msg, struct sockaddr_storage *dst) {
    Json::FastWriter writer;
    std::string str = writer.write(msg);
    size_t len = str.length();
    int result = sendto(sock, str.c_str(), len, 0, (struct sockaddr *)dst, sizeof(sockaddr_storage));
}

struct sockaddr_storage *SocketTransport::guid_lookup(std::string hex) {
    std::map<std::string, sockaddr_storage>::iterator it;
    it = known_guids.find(hex);
    if(it != known_guids.end()) {
        return &it->second;
    }
    return nullptr;
}

void SocketTransport::send(Message msg) {
    struct sockaddr_storage *dst = NULL;
    Path dst_path = Path(msg["dst"].asString());
    if(dst_path.is_broadcast) {
        dst = &broadcast;
    }
    else {
        dst = guid_lookup(msg["dst"].asString());
    }
    if(dst) {
        sendmsg(sock, msg, dst);
    }
    else {
        Result *result = Node::Single()->rpc("*/Routing", "who_has", msg["dst"]);
        result->when([=] (Json::Value value, bool success) {
            if(value.isString() && UUID::is(value.asString())) {
                struct sockaddr_storage *_dst = guid_lookup(value.asString());
                sendmsg(sock, msg, _dst);
            }
        });
    }

    //throw InvalidPath("Failed to send to destination " + msg["dst"].asString());

}

Message SocketTransport::recv() {
    char buffer[4096];
    struct sockaddr_storage from;
    uint from_size;
    Message output;

    from_size = sizeof(from);
    int recvd = recvfrom(this->sock, buffer, sizeof(buffer), MSG_DONTWAIT, (struct sockaddr *)&from, &from_size);
    if(recvd > 0) {
        output = Message::FromString(buffer, recvd);
        if(output.is_valid())
            known_guids[output["src"].asString()] = from;
        return output;
    }
    return output;
}