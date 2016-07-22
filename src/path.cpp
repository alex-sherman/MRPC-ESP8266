#include "path.h"
#include <stdlib.h>
#include <string.h>
#include <Arduino.h>

using namespace MRPC;

Path::Path(const char*path) {
    int len = strlen(path);
    int i;
    for(i = 0; i < len; i++) {
        if(path[i] == '.') 
            break;
    }
    if(i == len) return;
    char*_name = (char*)path;
    char*_method = _name + i + 1;
    switch(_name[0]) {
        case '/':
            _name++;
            i--;
            break;
        case '*':
            is_wildcard = true;
            break;
        default:
            is_uuid = true;
            break;
    }
    is_valid = true;

    memcpy(name, _name, i);
    name[i] = 0;
    strcpy(method, _method);
}

bool Path::match(const char*service_name, Service *service) {
    if(strcmp(method, service_name) != 0)
        return false;
    if(is_wildcard)
        return true;
    for(char *alias : service->aliases) {
        if(strcmp(alias, name))
            return true;
    }
    return false;
}