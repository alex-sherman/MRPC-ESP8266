#ifndef _JRPC_PATH_H_
#define _JRPC_PATH_H_
#include "service.h"
#include "mrpc.h"

namespace MRPC {
    class Service;
    class Path {
    public:
        bool is_wildcard = false;
        bool is_valid = false;
        bool is_uuid = false;
        char name[64];
        char method[64];

        Path(const char* path);
        bool match(Service *service);
    };
}

#endif