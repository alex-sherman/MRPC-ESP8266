#include "mrpc.h"

using namespace MRPC;

void Result::resolve(aJsonObject& value, bool success) {
    this->success = success;
    for(int i = 0; i < callbacks.size(); i++) {
        callbacks[i](value, success);
    }
}
void Result::when(Callback callback) {
    callbacks.push_back(callback);
}
