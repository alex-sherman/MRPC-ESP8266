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
    Serial.println("UDP Send begin");
    Json::println(msg, Serial);
    size_t len = Json::measure(msg);
    Serial.print("Length: ");
    Serial.println(len);
    char buffer[len];
    Json::dump(msg, buffer, sizeof(buffer));
    Serial.println(len);
    Serial.println(buffer);
    udp->beginPacket(address->ip, address->port); //NTP requests are to port 123
    udp->write(buffer, sizeof(buffer));
    udp->endPacket();
    Serial.println("UDP Send end");
}

void UDPTransport::send(Json::Object &msg) {
    Serial.print("Sending message to: ");
    struct UDPEndpoint *dst = NULL;
    Serial.println(msg["dst"].asString());
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
        Result *result = node->rpc("*/Routing", "who_has", msg["dst"]);
        result->when([=] (Json::Value value, bool success, void *data) {
            Json::Object &msg = *(Json::Object*)data;
            Serial.println("Got a routing response");
            if(value.isString() && UUID::is(value.asString())) {
                struct UDPEndpoint *_dst = known_guids.get(value.asString());
                if(_dst) {
                    sendmsg(&udp, msg, _dst);
                }
            }
        }, msg.clone());
    }
}

Json::Object &UDPTransport::recv() {
    Json::Object &output = *(new Json::Object());
    int cb = udp.parsePacket();
    char buffer[1024];
    if(cb > 0) {
        udp.read(buffer, 1023);
        buffer[cb + 1] = 0;
        Json::Value read = Json::parse(buffer);
        Serial.println("Parsed message");
        if(!read.isObject())
            return output;

        output = read.asObject();

        if(Message::is_valid(output)) {
            Serial.print("Message valid from: ");
            Serial.println(output["src"].asString());
            struct UDPEndpoint remote = {udp.remoteIP(), udp.remotePort()};
            known_guids.set(output["src"].asString(), remote);
        }
    }
    return output;
}