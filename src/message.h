#ifndef _MRPC_MESSAGE_H_
#define _MRPC_MESSAGE_H_

#include <json.h>

namespace MRPC {

    class Message {
    public:
        static int id;
        static Json::Object &Create(int id, const char* src, const char* dst);
        static Json::Object &Create(const char* src, const char* dst);
        static Json::Object &FromString(char *str);
        static bool is_request(Json::Object&);
        static bool is_response(Json::Object&);
        static bool is_valid(Json::Object&);

    };
}

#endif //_JRPC_MESSAGE_H_