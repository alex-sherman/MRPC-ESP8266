#ifndef _JRPC_PATH_H_
#define _JRPC_PATH_H_
#include <string>

namespace MRPC {
    class Path {
    public:
        Path(std::string path);
        bool is_broadcast;
        std::string path;
        std::string service;
    };
}

#endif