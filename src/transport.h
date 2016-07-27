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
        void send(Json::Object&, bool broadcast);
        Json::Value recv();
        struct UDPEndpoint *guid_lookup(const char *hex);
        uint16_t remote_port;
    private:
        struct UDPEndpoint broadcast;
        KeyValuePair<struct UDPEndpoint> last_sender;
        WiFiUDP udp;
    };
}