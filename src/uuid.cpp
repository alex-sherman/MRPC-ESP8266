#include "uuid.h"
#include <string.h>

using namespace MRPC;

UUID::UUID() {
    hex = "8159d51aeadb4bcd9a5b8119a486d851";
}
UUID::UUID(const char* hex) {
    this->hex = hex;
}
bool UUID::is(const char* hex) {
    return strlen(hex) == 32;
}