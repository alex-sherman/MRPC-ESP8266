#include "uuid.h"
#include <string.h>
#include <stdint.h>
#include <Arduino.h>

using namespace MRPC;

const char *hex_lookup = "0123456789abcdef";
class hex_char {
public:
    char hex[2];
};

hex_char byte_to_hex(uint8_t b) {
    uint8_t MS_nibble = b >> 4;
    uint8_t LS_nibble = b & 0x0f;
    char output[2];
    output[0] = hex_lookup[MS_nibble];
    output[1] = hex_lookup[LS_nibble];
    //Holy shit C++ sucks, can I seriously not return a char array any other way?
    //You can't intialize a character array by copy? Come on C++
    return hex_char{{output[0], output[1]}};
}

UUID::UUID() {
    randomSeed((int)RANDOM_REG32);
    for(int i = 0; i < 36; i+=2) {
        uint8_t b = random(256);
        if(i == 6) b = 0x40 | (b & 0x0F);
        if(i == 8) b = 0x80 | (b & 0x0F);
        hex_char h = byte_to_hex(b);
        this->chars[i] = h.hex[0];
        this->chars[i + 1] = h.hex[1];
    }
    this->chars[8] = '-';
    this->chars[13] = '-';
    this->chars[18] = '-';
    this->chars[23] = '-';
    this->chars[36] = 0;
}
UUID::UUID(const char* chars) {
    strncpy(this->chars, chars, sizeof(this->chars));
    this->chars[36] = 0;
}
bool UUID::is(const char* chars) {
    return strlen(chars) == 36;
}