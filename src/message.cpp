#include <string.h>
#include <stdlib.h>
#include "mrpc.h"
#include "message.h"

using namespace MRPC;

using namespace Json;

Json::Object Message::Create(int id, const char* src, const char* dst) {
    Json::Object msg = Create(src, dst);
    msg["id"] = Value::from_int(id);
    return msg;
}
Json::Object Message::Create(const char* src, const char* dst) {
    Json::Object msg;
    msg["src"] = Value::from_string(src);
    msg["dst"] = Value::from_string(dst);
    return msg;
}

bool Message::is_valid(Json::Object msg) {
    return msg["src"].isString() && msg["dst"].isString() && (Message::is_response(msg) || Message::is_request(msg));
}

bool Message::is_response(Json::Object msg) {
    return msg["id"].isInt() && (msg["result"].isString() || msg["error"].isString());
}
bool Message::is_request(Json::Object msg) {
    return msg["procedure"].isString();
}