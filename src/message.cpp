#include <string.h>
#include <stdlib.h>
#include "mrpc.h"
#include "message.h"

using namespace MRPC;

aJsonObject &Message::Create(int id, const char* src, const char* dst) {
    aJsonObject &msg = Create(src, dst);
    msg.set("id", id);
    return msg;
}
aJsonObject &Message::Create(const char* src, const char* dst) {
    aJsonObject &msg = *aJson.createObject();
    msg.set("src", src);
    msg.set("dst", dst);
    return msg;
}

bool Message::is_valid(aJsonObject & msg) {
    if(msg.isNull()) return false;
    return !msg["src"].isNull() && !msg["dst"].isNull() && (Message::is_response(msg) || Message::is_request(msg));
}

bool Message::is_response(aJsonObject & msg) {
    return !msg["id"].isNull() && (!msg["result"].isNull() || !msg["error"].isNull());
}
bool Message::is_request(aJsonObject & msg) {
    return !msg["procedure"].isNull();
}