#ifndef _MRPC_SERVICE_H_
#define _MRPC_SERVICE_H_

#ifdef ARDUINO
    #include <vector>
    #include <map>
    #include <ArduinoJson.h>
#endif

namespace MRPC {

    class Node;
    class Service;
    typedef JsonVariant (*ServiceMethod)(Service* self, const JsonVariant& value, StaticJsonBuffer<2048>* messageBuffer, bool& success);
    typedef JsonVariant (*PublisherMethod)(Service*, StaticJsonBuffer<2048>*);

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
        Node *node;
        Service();
        void add_method(const char*, ServiceMethod method);
        ServiceMethod get_method(const char*);
        void add_publisher(const char* name, PublisherMethod method, const char* procedure, int interval);
        void update(uint64_t time);
        std::map<const char*, Publisher*> publishers;
        std::map<const char*, ServiceMethod> methods;
    private:
        JsonObject* storage;
    };
}

#endif