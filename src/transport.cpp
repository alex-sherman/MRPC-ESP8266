#include "mrpc.h"
#include <string.h>
#include <Arduino.h>

using namespace MRPC;

bool Transport::poll() {
    bool ret = true;
    Json::Object msg = recv();
    if(!Message::is_valid(msg))
        ret = false;
    else {
        Serial.println("Got message");
        Json::print(msg, Serial);
        Serial.println();
        if(strcmp(msg["src"].asString(), node->guid.hex))
            node->on_recv(msg);
    }
    return ret;
}