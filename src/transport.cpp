#include "mrpc.h"
#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <Arduino.h>

using namespace MRPC;

bool UDPTransport::poll() {
    bool ret = true;
    struct UDPEndpoint from;
    for(int i = 0; i < 5 && ret; i++) {
        Json::Value msg = recv(&from);
        if(!msg.isObject() || !Message::is_valid(msg.asObject()))
            ret = false;
        else {
            if(strcmp(msg.asObject()["src"].asString(), MRPC::guid().chars)) {

                if(Message::is_valid(msg.asObject())) {
                    strncpy(last_sender.key, msg.asObject()["src"].asString(), sizeof(last_sender.key));
                    last_sender.value = from;
                }

                MRPC::on_recv(msg.asObject(), from);
            }
        }
        msg.free_parsed();
        yield();
    }
    return ret;
}

UDPTransport::UDPTransport()
: UDPTransport(0) {
}

UDPTransport::UDPTransport(int local_port) {
    udp.begin(local_port);
    remote_port = local_port;
    broadcast.ip = IPAddress(3, 255, 255, 255);
    broadcast.port = remote_port;
}

void UDPTransport::senddst(Json::Object &msg, struct UDPEndpoint *address) {
    Serial.println(address->ip);
    size_t len = Json::measure(msg);
    char buffer[len];
    Json::dump(msg, buffer, sizeof(buffer));
    udp.beginPacket(address->ip, address->port); //NTP requests are to port 123
    udp.write(buffer, sizeof(buffer));
    udp.endPacket();
}

void UDPTransport::send(Json::Object &msg) {
    struct UDPEndpoint *dst = strcmp(msg["dst"].asString(), last_sender.key) == 0 ?
        &last_sender.value : &this->broadcast;
    senddst(msg, dst);
}

Json::Value UDPTransport::recv(UDPEndpoint *from) {
    Json::Value output;
    int cb = udp.parsePacket();
    char buffer[1024];
    if(cb > 0) {
        udp.read(buffer, 1023);
        from->ip = udp.remoteIP();
        from->port = udp.remotePort();
        Serial.print("Got from ");
        Serial.println(from->ip);
        buffer[cb + 1] = 0;
        Json::Value read = Json::parse(buffer);
        if(!read.isObject()) {
            read.free_parsed();
            return output;
        }

        output = read.asObject();

    }
    return output;
}