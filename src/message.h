#ifndef _MRPC_MESSAGE_H_
#define _MRPC_MESSAGE_H_

#include <aJSON.h>

namespace MRPC {

    class Message {
    public:
        static aJsonObject & Create(int id, const char* src, const char* dst);
        static aJsonObject & Create(const char* src, const char* dst);
        static aJsonObject & FromString(char *str, size_t size);
        static bool is_request(aJsonObject &);
        static bool is_response(aJsonObject &);
        static bool is_valid(aJsonObject &);

    };
}

#endif //_JRPC_MESSAGE_H_