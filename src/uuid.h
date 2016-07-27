#ifndef _MRPC_UUID_H_
#define _MRPC_UUID_H_

namespace MRPC {
    class UUID {
    public:
        static bool is(const char* test);
        UUID(const char* hex);
        UUID();
        char hex[33];
    };
}

#endif