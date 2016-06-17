#ifndef _MRPC_UUID_H_
#define _MRPC_UUID_H_

#include <string>

namespace MRPC {
    class UUID {
    public:
        static bool is(std::string test);
        UUID(std::string hex);
        UUID();
        std::string hex;
    };
}

#endif