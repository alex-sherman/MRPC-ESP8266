#ifndef _MRPC_MESSAGE_H_
#define _MRPC_MESSAGE_H_

namespace MRPC {

    class Message : public Json::Value {
    public:
        static Message Create(int id, std::string src, std::string dst);
        static Message Create(std::string src, std::string dst);
        static Message FromString(char *str, size_t size);
        bool is_request();
        bool is_response();
        bool is_valid();
    };
}

#endif //_JRPC_MESSAGE_H_