#ifndef _MRPC_SERVICE_H_
#define _MRPC_SERVICE_H_

#include <json.h>
#include "path.h"

namespace MRPC {

    class Service;
    typedef Json::Value (*ServiceMethod)(Service* self, Json::Value& value, bool& success);
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
        Service(const char *method_name, ServiceMethod method) {
            strncpy(this->method_name, method_name, sizeof(this->method_name));
            this->method = method;
        }
        Service& respond(const char* alias) {
            char *buf = (char*)malloc(strlen(alias));
            strcpy(buf, alias);
            aliases.append(buf);
            return *this;
        }
        ServiceMethod method;
        char method_name[64];
        AList<char *> aliases;
    };
}

#endif