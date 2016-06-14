#ifndef _JRPC_MESSAGE_H_
#define _JRPC_MESSAGE_H_

namespace MRPC {

    class Message : public Json::Value {
    public:
        static Message *FromString(char *str, size_t size);
        bool is_request();
        bool is_response();
        bool is_valid();
    };
}

#endif //_JRPC_MESSAGE_H_