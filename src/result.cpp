#include "mrpc.h"

using namespace MRPC;

void Result::resolve(Json::Value value, bool success) {
    this->success = success;
    this->callback(value, success, data);
}
void Result::when(Callback callback, Json::Value data) {
    this->data = data;
    this->callback = callback;
}
