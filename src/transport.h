#pragma once

namespace MRPC {
    class Node;
    class Transport {
    public:
        bool poll();
        void close();
        virtual void send(Json::Object&, bool broadcast) = 0;
        virtual Json::Value recv() = 0;
    };
    struct UDPEndpoint {
        IPAddress ip;
        uint16_t port;
    };
    class UDPTransport : public Transport {
    public:
        UDPTransport();
        UDPTransport(int local_port);
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