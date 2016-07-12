#ifndef _MRPC_MESSAGE_H_
#define _MRPC_MESSAGE_H_

#include <ArduinoJson.h>

namespace MRPC {

    class Message {
    public:
        static JsonObject& Create(int id, const char* src, const char* dst, StaticJsonBuffer<2048>* messageBuffer);
        static JsonObject& Create(const char* src, const char* dst, StaticJsonBuffer<2048>* messageBuffer);
        static JsonObject& FromString(char *str, size_t size, StaticJsonBuffer<2048>* messageBuffer);
        static bool is_request(JsonObject&);
        static bool is_response(JsonObject&);
        static bool is_valid(JsonObject&);

    };
}

#endif //_JRPC_MESSAGE_H_