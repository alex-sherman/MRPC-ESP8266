#include "path.h"
#include <stdlib.h>
#include <string.h>
#include <Arduino.h>

using namespace MRPC;

Path::Path(const char* path) {
    strncpy(this->path, path, sizeof(this->path));
    char *_path = (char*)path;
    if(_path[0] == '*') {
        is_broadcast = true;
        _path++;
    }
    if(_path[0] == '/')
        _path++;
    strncpy(identifiers, _path, sizeof(identifiers));
    identifier_count = 1;
    identifier_offsets[0] = 0;
    int len = strlen(_path);
    len = len > 255 ? 255 : len;
    for(int i = 0; i <= len; i++) {
        if(i == len + 1 || identifiers[i] == '/') {
            identifiers[i] = 0;
            identifier_count++;
            identifier_offsets[identifier_count] = i + 1;
        }
    }
}
bool Path::match(Path &other) {
    int i;
    for(i = 0; i < identifier_count && i < other.identifier_count; i++) {
        if(strcmp((*this)[i], other[i]) != 0)
            return false;
    }
    return i == identifier_count && i == other.identifier_count;
}

Path Path::concat(Path &suffix) {
    char buf[256];
    strcpy(buf, path);
    strcat(buf, suffix.path);
    return Path(buf);
}