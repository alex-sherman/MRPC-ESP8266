#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <json/json.h>
#include <iostream>
#include "mrpc.h"
#include "message.h"

using namespace MRPC;

Message Message::Create(int id, std::string src, std::string dst) {
    Message msg = Create(src, dst);
    msg["id"] = id;
    return msg;
}
Message Message::Create(std::string src, std::string dst) {
    Message msg = Message();
    msg["src"] = src;
    msg["dst"] = dst;
    return msg;
}

Message Message::FromString(char *str, size_t size) {
    Message msg;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( str, msg );
    if ( !parsingSuccessful )
    {
        // report to the user the failure and their locations in the document.
        std::cout  << "Failed to parse configuration\n"
                   << reader.getFormattedErrorMessages();
    }
    return msg;
}

bool Message::is_valid() {
    if(!isObject())
        return false;
    return isMember("src") && isMember("dst") && (is_response() || is_request());
}

bool Message::is_response() {
    return isMember("id") && (isMember("result") || isMember("error"));
}
bool Message::is_request() {
    return isMember("procedure");
}