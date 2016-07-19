#pragma once
#include "json.h"
#include <functional>

namespace MRPC {
    typedef std::function<void(Json::Value, bool, Json::Value)> Callback;
    class Result {
        
    public:
        Result() { timestamp = millis(); }
        long timestamp;
        bool stale() { return (millis() - timestamp) > 3000; }
        void resolve(Json::Value, bool success);
        void when(Callback callback, Json::Value data);
        Callback callback;
        bool completed;
        bool success;
        Json::Value data;
    };
}