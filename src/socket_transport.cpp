#include "mrpc.h"
#include <stdlib.h>
#include <string.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <map>

using namespace MRPC;

UDPTransport::UDPTransport()
: UDPTransport(0) {
}

UDPTransport::UDPTransport(int local_port) {
    udp.begin(local_port);
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

void UDPTransport::send(JsonObject& msg, StaticJsonBuffer<2048>* jsonBuffer) {
    Serial.println("Sent a message");
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
        int offset = ((uint8_t*)&msg - (uint8_t*)jsonBuffer);
        StaticJsonBuffer<2048> buffer_copy = *jsonBuffer;
        Serial.println("Derpy copying message: " + (offset));
        Result *result = node->rpc("*/Routing", "who_has", msg["dst"], jsonBuffer);
        result->when([=] (JsonVariant value, bool success) {
            Serial.println("Got a routing response");
            if(value.is<const char*>() && UUID::is(value.as<const char*>())) {
                struct UDPEndpoint *_dst = known_guids.get(value.as<const char*>());
                sendmsg(&udp, *(JsonObject*)((uint8_t *)&buffer_copy)[offset], _dst);
            }
        });
    }
}

bool UDPTransport::recv(char buffer[1024]) {
    int cb = udp.parsePacket();
    if(cb > 0) {
        udp.read(buffer, 1023);
        buffer[cb + 1] = 0;
        StaticJsonBuffer<1024> jsonBuffer;
        JsonObject& output = jsonBuffer.parseObject(buffer);
        Serial.println("Parsed message");
        output.printTo(Serial);

        if(Message::is_valid(output)) {
            Serial.println("Message valid");
            Serial.println(output["src"].as<const char*>());
            struct UDPEndpoint remote = {udp.remoteIP(), udp.remotePort()};
            Serial.println("HERP");
            known_guids.set(output["src"].as<const char*>(), remote);
            Serial.println("Returning");
            return true;
        }
    }
    return false;
}