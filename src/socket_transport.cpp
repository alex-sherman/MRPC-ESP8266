#include "mrpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

using namespace MRPC;

SocketTransport::SocketTransport()
: SocketTransport(0) {
}

SocketTransport::SocketTransport(int local_port) {
    this->sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in myaddr;
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(local_port);

    if (bind(this->sock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
    }
}

void SocketTransport::send(Message msg) {
    struct sockaddr_storage *dst = NULL;
    std::map<std::string, sockaddr_storage>::iterator it;
    it = known_guids.find(msg["dst"].asString());
    if(it != known_guids.end()) {
        dst = &it->second;
    }
    if(dst) {
        Json::FastWriter writer;
        std::string str = writer.write(msg);
        size_t len = str.length();
        int result = sendto(sock, str.c_str(), len, 0, (struct sockaddr *)dst, sizeof(sockaddr_storage));
    }
}

Message SocketTransport::recv() {
    char buffer[4096];
    struct sockaddr_storage from;
    uint from_size;
    Message output;

    while(1) {
        from_size = sizeof(from);
        int recvd = recvfrom(this->sock, buffer, sizeof(buffer), MSG_DONTWAIT, (struct sockaddr *)&from, &from_size);
        if(recvd > 0) {
            output = Message::FromString(buffer, recvd);
            if(output.is_valid())
                known_guids[output["src"].asString()] = from;
            return output;
        }
    }
    return output;
}