#ifndef _MRPC_SERVICE_H_
#define _MRPC_SERVICE_H_

#include <json.h>
#include "path.h"

namespace MRPC {

    Json::Object &settings();

    class Service;
    typedef Json::Value (*ServiceMethod)(Json::Value& value, bool& success);
    typedef Json::Value (*PublisherMethod)();

    class Publisher {
    public:
        Publisher(PublisherMethod method, const char* path, uint interval)
        : method(method), interval(interval) {
            strncpy(this->path, path, sizeof(this->path));
        }
        PublisherMethod method;
        char path[64];
        uint interval;
        uint64_t last_called;
    };

    class Service {
    public:
        Service(const char* name, ServiceMethod method) {
            strncpy(this->name, name, sizeof(this->name));
            this->method = method;
        }
        ServiceMethod method;
        char name[64];
    };
}

#endif