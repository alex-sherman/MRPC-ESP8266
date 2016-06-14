#include "mrpc.h"
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

int SocketTransport::recv(char *buffer, size_t buffer_size) {
    struct sockaddr_in from;
    uint from_size;

    while(1) {
        from_size = sizeof(from);
        return recvfrom(this->sock, buffer, buffer_size, MSG_DONTWAIT, (struct sockaddr *)&from, &from_size);
    }
}