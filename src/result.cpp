#include "mrpc.h"

using namespace MRPC;

void Result::resolve(Json::Value value, bool success) {
    this->success = success;
    for(int i = 0; i < callbacks.count; i++) {
        callbacks[i](value, success);
    }
}
void Result::when(Callback callback) {
    callbacks.append(callback);
}
