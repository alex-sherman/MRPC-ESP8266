#include "mrpc.h"

using namespace MRPC;

void Result::resolve(Json::Value value, bool failure) {
    for(int i = 0; i < callbacks.size(); i++) {
        callbacks[i](value);
    }
}
