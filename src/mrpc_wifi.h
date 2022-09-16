#ifndef _MRPC_MESH_H_
#define _MRPC_MESH_H_
#include "mrpc.h"

namespace MRPC {
    class MRPCWifi {
    public:
        void init();
        void poll();
        IPAddress localIP();
        static IPAddress ap_broadcast() {
            return IPAddress((uint32_t)ap_addr | ~(uint32_t)ap_netmask);
        }
        static IPAddress client_broadcast() {
            return IPAddress((uint32_t)client_addr | ~(uint32_t)client_netmask);
        }
    private:
        void _onDisconnect();
        void _onConnect(Json::Object &);
        void _onScanDone(Json::Object &);
        void setupWiFiAP(const char* password);
        int failures;
        static IPAddress client_addr;
        static IPAddress client_netmask;
        static IPAddress ap_addr;
        static IPAddress ap_netmask;
    };
}
#endif