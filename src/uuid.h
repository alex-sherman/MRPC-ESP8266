#ifndef _MRPC_UUID_H_
#define _MRPC_UUID_H_

namespace MRPC {
    class UUID {
    public:
        static bool is(const char* test);
        UUID(const char* chars);
        UUID();
        char chars[37];
    };
}

#endif