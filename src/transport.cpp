#include "mrpc.h"
#include <iostream>

using namespace MRPC;

bool Transport::poll() {
    Message *msg = NULL;
    char buffer[4096];
    int size = recv(buffer, sizeof(buffer));
    bool valid = false;
    if(size > 0) {
        msg = Message::FromString(buffer, size);
        valid = msg->is_valid();
        if(msg && valid) {
            Node::Single()->on_recv(msg);
        }
        if(msg)
            delete msg;
    }
    return valid;
}