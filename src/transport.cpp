#include "jrpc.h"

using namespace MRPC;

void TransportThread(Transport *transport) {
    while(true) {
        Message *msg = transport->recv();
        if(msg) {

        }
    }
}

void Transport::begin() {
    recv_thread = std::thread(TransportThread, this);
    recv_thread.detach();
}