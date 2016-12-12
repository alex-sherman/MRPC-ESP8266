#pragma once

namespace MRPC {
    struct UDPEndpoint {
        IPAddress ip;
        uint16_t port;
    };
    class UDPTransport {
    public:
        UDPTransport();
        UDPTransport(int local_port);
        bool poll();
        void close();
        void send(Json::Object& msg);
        void senddst(Json::Object &msg, struct UDPEndpoint *address);
        Json::Value recv(UDPEndpoint *);
        struct UDPEndpoint *guid_lookup(const char *hex);
        uint16_t remote_port;
    private:
        struct UDPEndpoint broadcast;
        KeyValuePair<struct UDPEndpoint> last_sender;
        WiFiUDP udp;
    };
}