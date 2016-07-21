#ifndef _MRPC_SERVICE_H_
#define _MRPC_SERVICE_H_

#include <json.h>
#include "path.h"

namespace MRPC {

    class Service;
    typedef Json::Value (*ServiceMethod)(Service* self, Json::Value& value, bool& success);
    typedef Json::Value (*PublisherMethod)(Service*);

    class Publisher {
    public:
        Publisher(PublisherMethod method, const char* procedure) :
            Publisher(method, procedure, 0) { };
        Publisher(PublisherMethod, const char* procedure, uint);
        Publisher() { }
        PublisherMethod method;
        const char* procedure;
        uint interval;
        uint64_t last_called; 
    };

    class Service {
    public:
        Service(const char *name);
        void add_method(const char*, ServiceMethod method);
        ServiceMethod get_method(const char*);
        void add_publisher(const char* name, PublisherMethod method, int interval);
        void update(uint64_t time);
        AMap<Publisher*> publishers;
        AMap<ServiceMethod> methods;
        char name[64];
        Path path;
    private:
        Json::Object storage;
    };
}

#endif