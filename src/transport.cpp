#include "mrpc.h"
#include <string.h>
#include <Arduino.h>

using namespace MRPC;

bool Transport::poll() {
    bool ret = true;
    for(int i = 0; i < 5 && ret; i++) {
        Json::Value msg = recv();
        if(!msg.isObject() || !Message::is_valid(msg.asObject()))
            ret = false;
        else {
            if(strcmp(msg.asObject()["src"].asString(), MRPC::guid().hex))
                MRPC::on_recv(msg.asObject());
        }
        msg.free_parsed();
        yield();
    }
    return ret;
}