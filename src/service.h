#ifndef _MRPC_SERVICE_H_
#define _MRPC_SERVICE_H_

#include <json.h>
#include "path.h"

namespace MRPC {

    Json::Object &settings();

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
        Service(const char* name, ServiceMethod method) {
            strncpy(this->name, name, sizeof(this->name));
            configure();
            this->method = method;
        }
        Json::Object &configure() {
            if(!settings()["services"].isObject())
                settings()["services"] = new Json::Object();
            Json::Object &services = settings()["services"].asObject();
            if(!services[name].isObject())
                services[name] = new Json::Object();
            Json::Object &service = services[name].asObject();
            if(!service["name"].isString())
                service["name"] = name;
            method_name = service["name"].asString();
            if(!service["aliases"].isArray())
                service["aliases"] = new Json::Array();
            aliases = &service["aliases"].asArray();
            return services[name].asObject();
        }
        ServiceMethod method;
        char name[64];
        const char *method_name;
        Json::Array *aliases;
    };
}

#endif