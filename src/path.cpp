#include "path.h"
#include <stdlib.h>
#include <string.h>

using namespace MRPC;

Path::Path(const char* path) {
    is_broadcast = path[0] == '*';
    int path_len = strlen(path);
    if(is_broadcast)
        memcpy(service, path + 1, path_len);
    else
        memcpy(service, path, path_len + 1);
    memcpy(this->path, path, path_len + 1);
}