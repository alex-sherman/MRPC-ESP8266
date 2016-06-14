#include "mrpc.h"
#include <iostream>

using namespace MRPC;

bool Transport::poll() {
    Message msg = recv();
    if(!msg)
        return false;
    if(msg.is_valid())
        Node::Single()->on_recv(msg);
    return true;
}