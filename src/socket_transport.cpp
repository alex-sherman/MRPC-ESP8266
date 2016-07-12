#include "mrpc.h"
#include <stdlib.h>
#include <string.h>
#include <ArduinoJson.h>
#include <Arduino.h>

using namespace MRPC;

UDPTransport::UDPTransport()
: UDPTransport(0) {
}

UDPTransport::UDPTransport(int local_port) {
    udp.begin(local_port);
    known_guids = std::map<const char*, struct UDPEndpoint>();
    remote_port = local_port;
    broadcast.ip = IPAddress(255, 255, 255, 0);
    broadcast.port = remote_port;
}

void sendmsg(WiFiUDP *udp, const JsonObject& msg, struct UDPEndpoint *address) {
    Serial.println("UDP Send begin");
    size_t len = msg.measureLength();
    char buffer[len + 1];
    msg.printTo(buffer, sizeof(buffer));
    udp->beginPacket(address->ip, address->port); //NTP requests are to port 123
    udp->write(buffer, sizeof(buffer));
    udp->endPacket();
    Serial.println("UDP Send end");
}

struct UDPEndpoint *UDPTransport::guid_lookup(const char *hex) {
    std::map<const char*, struct UDPEndpoint>::iterator it;
    it = known_guids.find(hex);
    if(it != known_guids.end()) {
        return &it->second;
    }
    return nullptr;
}

void UDPTransport::send(JsonObject& msg, StaticJsonBuffer<2048>* jsonBuffer) {
    Serial.println("Sent a message");
    struct UDPEndpoint *dst = NULL;
    Path dst_path = Path(msg["dst"].asString());
    if(dst_path.is_broadcast) {
        dst = &broadcast;
    }
    else {
        dst = guid_lookup(msg["dst"].asString());
    }
    if(dst) {
        sendmsg(&udp, msg, dst);
    }
    else {
        int offset = ((uint8_t*)&msg - (uint8_t*)jsonBuffer);
        StaticJsonBuffer<2048> buffer_copy = *jsonBuffer;
        Serial.println("Derpy copying message: " + (offset));
        Result *result = node->rpc("*/Routing", "who_has", msg["dst"], jsonBuffer);
        result->when([=] (JsonVariant value, bool success) {
            Serial.println("Got a routing response");
            if(value.is<const char*>() && UUID::is(value.as<const char*>())) {
                struct UDPEndpoint *_dst = guid_lookup(value.as<const char*>());
                sendmsg(&udp, *(JsonObject*)((uint8_t *)&buffer_copy)[offset], _dst);
            }
        });
    }
}

bool UDPTransport::recv(char buffer[1024]) {
    int cb = udp.parsePacket();
    if(cb > 0) {
        udp.read(buffer, 1023);
        StaticJsonBuffer<1024> jsonBuffer;
        JsonObject& output = jsonBuffer.parseObject(buffer);
        if(Message::is_valid(output)) {
            struct UDPEndpoint remote = {udp.remoteIP(), udp.remotePort()};
            known_guids[output["src"].as<const char*>()] = remote;
            return true;
        }
    }
    return false;
}