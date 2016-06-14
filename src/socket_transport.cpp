#include "jrpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

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

void SocketTransport::send(Message *msg) {

}

Message *SocketTransport::recv() {
    struct sockaddr_in from;
    uint from_size;
    char buffer[4096];

    while(1) {
        from_size = sizeof(from);
        int size = recvfrom(this->sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&from, &from_size);
        if(size < 0) {
            perror("\n");
        }
        else
            printf("Size: %d\n", size);
        return NULL;
    }
}