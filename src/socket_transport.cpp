#include "mrpc.h"
#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <Arduino.h>
#include <map>

using namespace MRPC;

UDPTransport::UDPTransport()
: UDPTransport(0) {
}

UDPTransport::UDPTransport(int local_port) {
    udp.begin(local_port);
    remote_port = local_port;
    broadcast.ip = IPAddress(255, 255, 255, 255);
    broadcast.port = remote_port;
}

void sendmsg(WiFiUDP *udp, Json::Object &msg, struct UDPEndpoint *address) {
    size_t len = Json::measure(msg);
    char buffer[len];
    Json::dump(msg, buffer, sizeof(buffer));
    udp->beginPacket(address->ip, address->port); //NTP requests are to port 123
    udp->write(buffer, sizeof(buffer));
    udp->endPacket();
}

void UDPTransport::send(Json::Object &msg) {
    struct UDPEndpoint *dst = NULL;
    Path dst_path = Path(msg["dst"].asString());
    if(dst_path.is_broadcast) {
        dst = &broadcast;
    }
    else {
        dst = known_guids.get(msg["dst"].asString());
    }
    if(dst) {
        sendmsg(&udp, msg, dst);
    }
    else {
        Result *result = rpc("*/Routing", "who_has", msg["dst"]);
        result->when([=] (Json::Value value, bool success, Json::Value data) {
            Json::Object &msg = data.asObject();
            if(value.isString() && UUID::is(value.asString())) {
                struct UDPEndpoint *_dst = known_guids.get(value.asString());
                if(_dst) {
                    sendmsg(&udp, msg, _dst);
                }
            }
        }, *msg.clone());
    }
}

Json::Value UDPTransport::recv() {
    Json::Value output;
    int cb = udp.parsePacket();
    char buffer[1024];
    if(cb > 0) {
        udp.read(buffer, 1023);
        buffer[cb + 1] = 0;
        Json::Value read = Json::parse(buffer);
        if(!read.isObject()) {
            read.free_parsed();
            return output;
        }

        output = read.asObject();

        if(Message::is_valid(output.asObject())) {
            struct UDPEndpoint remote = {udp.remoteIP(), udp.remotePort()};
            known_guids.set(output.asObject()["src"].asString(), remote);
        }
    }
    return output;
}