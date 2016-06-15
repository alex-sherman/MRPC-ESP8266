#ifndef _MRPC_SERVICE_H_
#define _MRPC_SERVICE_H_

#include <json/json.h>

namespace MRPC {

    class Service;
    typedef Json::Value (*ServiceMethod)(Service*, Json::Value, Json::Value);
    typedef Json::Value (*PublisherMethod)(void);

    class Publisher {
    public:
        Publisher(PublisherMethod method, std::string path, std::string procedure) :
            Publisher(method, path, procedure, 0) { };
        Publisher(PublisherMethod, std::string path, std::string procedure, uint);
        Publisher() { }
        PublisherMethod method;
        std::string path;
        std::string procedure;
        uint interval;
        uint64_t last_called; 
    };

    class Service {
    public:
        Service();
        void add_method(std::string, ServiceMethod method);
        ServiceMethod get_method(std::string);
        void add_publisher(std::string name, PublisherMethod method, std::string procedure, int interval);
        Json::Value storage;
        void update(uint64_t time);
        std::map<std::string, Publisher*> publishers;
        std::map<std::string, ServiceMethod> methods;
    };
}

#endif