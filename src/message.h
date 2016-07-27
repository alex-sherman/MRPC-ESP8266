#ifndef _MRPC_MESSAGE_H_
#define _MRPC_MESSAGE_H_

#include <json.h>

namespace MRPC {

    class Message {
    public:
        static int id;
        static Json::Object &Create(int id, const char* src, const char* dst) {
            Json::Object &msg = Create(src, dst);
            msg["id"] = id;
            return msg;
        }
        static Json::Object &Create(const char* src, const char* dst) {
            Json::Object &msg = *(new Json::Object());
            msg["src"] = src;
            msg["dst"] = dst;
            return msg;
        }
        static bool is_request(Json::Object& msg) {
            return !Message::is_response(msg);
        }
        static bool is_response(Json::Object& msg) {
            return msg["id"].isInt() && (msg["result"].isString() || msg["error"].isString());
        }
        static bool is_valid(Json::Object& msg) {
            return msg["src"].isString() && msg["dst"].isString() && (Message::is_response(msg) || Message::is_request(msg));
        }

    };
}

#endif //_JRPC_MESSAGE_H_