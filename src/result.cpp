#include "mrpc.h"

using namespace MRPC;

void Result::resolve(Json::Value value, bool success) {
    this->success = success;
    for(int i = 0; i < callbacks.size(); i++) {
        callbacks[i](value, success, data);
    }
}
void Result::when(Callback callback, void *data) {
    this->data = data;
    callbacks.append(callback);
}
