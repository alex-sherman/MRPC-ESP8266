#include "uuid.h"

using namespace MRPC;

UUID::UUID() {
    hex = "8159d51aeadb4bcd9a5b8119a486d851";
}
UUID::UUID(std::string hex) {
    this->hex = hex;
}
bool UUID::is(std::string hex) {
    return hex.length() == 32;
}