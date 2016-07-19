#include "mrpc.h"
#include <string.h>
#include <Arduino.h>

using namespace MRPC;

bool Transport::poll() {
    bool ret = true;
    Json::Value msg = recv();
    if(!msg.isObject() || !Message::is_valid(msg.asObject()))
        ret = false;
    else {
        if(strcmp(msg.asObject()["src"].asString(), node->guid.hex))
            node->on_recv(msg.asObject());
    }
    msg.free_parsed();
    return ret;
}