#include "mrpc.h"
#include <string.h>
#include <Arduino.h>

using namespace MRPC;

bool Transport::poll() {
    StaticJsonBuffer<2048> jsonBuffer;
    char buffer[1024];
    if(!recv(buffer)) return false;
    Serial.println(buffer);
    JsonObject& msg = jsonBuffer.parseObject(buffer);
    if(!Message::is_valid(msg))
        return false;
    Serial.println("Got message");
    msg.printTo(Serial);
    Serial.println();
    if(strcmp(msg["src"].as<const char*>(), node->guid.hex))
        node->on_recv(msg, &jsonBuffer);
    return true;
}