#include "path.h"

using namespace MRPC;

Path::Path(std::string path) {
    is_broadcast = path.at(0) == '*';
    if(is_broadcast)
        service = path.substr(1, path.length());
    else
        service = path.substr(0, path.length());
    this->path = path.substr(0, path.length());
}