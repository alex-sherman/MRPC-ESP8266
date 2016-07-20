#ifndef _MRPC_SERVICE_H_
#define _MRPC_SERVICE_H_

#include <json.h>

namespace MRPC {

    class Service;
    typedef Json::Value (*ServiceMethod)(Service* self, Json::Value& value, bool& success);
    typedef Json::Value (*PublisherMethod)(Service*);

    class Publisher {
    public:
        Publisher(PublisherMethod method, const char* path, const char* procedure) :
            Publisher(method, path, procedure, 0) { };
        Publisher(PublisherMethod, const char* path, const char* procedure, uint);
        Publisher() { }
        PublisherMethod method;
        const char* path;
        const char* procedure;
        uint interval;
        uint64_t last_called; 
    };

    class Service {
    public:
        Service();
        void add_method(const char*, ServiceMethod method);
        ServiceMethod get_method(const char*);
        void add_publisher(const char* name, PublisherMethod method, const char* procedure, int interval);
        void update(uint64_t time);
        AMap<Publisher*> publishers;
        AMap<ServiceMethod> methods;
    private:
        Json::Object storage;
    };
}

#endif