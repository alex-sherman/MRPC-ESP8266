#ifndef _MRPC_UUID_H_
#define _MRPC_UUID_H_

#include <string>

namespace MRPC {
    class UUID {
    public:
        UUID(std::string hex);
        UUID();
        std::string hex;
    };
}

#endif