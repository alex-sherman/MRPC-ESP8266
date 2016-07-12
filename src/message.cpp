#include <string.h>
#include <stdlib.h>
#include "mrpc.h"
#include "message.h"

using namespace MRPC;

JsonObject& Message::Create(int id, const char* src, const char* dst, StaticJsonBuffer<2048>* messageBuffer) {
    JsonObject& msg = Create(src, dst, messageBuffer);
    msg["id"] = id;
    return msg;
}
JsonObject& Message::Create(const char* src, const char* dst, StaticJsonBuffer<2048>* messageBuffer) {
    JsonObject& msg = messageBuffer->createObject();
    msg["src"] = src;
    msg["dst"] = dst;
    return msg;
}

bool Message::is_valid(JsonObject& msg) {
    if(!msg.success()) return false;
    return msg.get<const char *>("src") != NULL && msg.get<const char *>("dst") != NULL && (Message::is_response(msg) || Message::is_request(msg));
}

bool Message::is_response(JsonObject& msg) {
    return msg.get<const char *>("id") != NULL && (msg.get<const char *>("result") != NULL || msg.get<const char *>("error") != NULL);
}
bool Message::is_request(JsonObject& msg) {
    return msg.get<const char *>("procedure") != NULL;
}