#ifndef _MRPC_SERVICE_H_
#define _MRPC_SERVICE_H_

#include <json/json.h>

namespace MRPC {
    typedef Json::Value *(*ServiceMethod)(Json::Value, Json::Value);
    class Service {
    public:
        void add_method(std::string, ServiceMethod method);
        ServiceMethod get_method(std::string);
    private:
        std::map<std::string, ServiceMethod> methods;
    };
}

#endif