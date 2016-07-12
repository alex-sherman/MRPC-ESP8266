#ifndef _JRPC_PATH_H_
#define _JRPC_PATH_H_

namespace MRPC {
    class Path {
    public:
        Path(const char* path);
        bool is_broadcast;
        char path[256];
        char service[256];
    };
}

#endif