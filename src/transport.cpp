#include "mrpc.h"
#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <Arduino.h>

using namespace MRPC;

bool UDPTransport::poll() {
    bool ret = true;
    for(int i = 0; i < 5 && ret; i++) {
        Json::Value msg = recv();
        if(!msg.isObject() || !Message::is_valid(msg.asObject()))
            ret = false;
        else {
            if(strcmp(msg.asObject()["src"].asString(), MRPC::guid().chars))
                MRPC::on_recv(msg.asObject());
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

void UDPTransport::send(Json::Object &msg, bool broadcast) {
    struct UDPEndpoint *dst = strcmp(msg["dst"].asString(), last_sender.key) == 0 ?
        &last_sender.value : &this->broadcast;
    sendmsg(&udp, msg, dst);
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
            strncpy(last_sender.key, output.asObject()["src"].asString(), sizeof(last_sender.key));
            last_sender.value.ip = udp.remoteIP();
            last_sender.value.port = udp.remotePort();
        }
    }
    return output;
}