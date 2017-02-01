#ifndef _MRPC_MESH_H_
#define _MRPC_MESH_H_
#include "mrpc.h"
#include <ESP8266WiFi.h>

namespace MRPC {
    class MRPCWifi {
    public:
        void poll();
        static bool is_client(IPAddress addr) {
            return ((uint32_t)addr & (uint32_t)client_netmask) == ((uint32_t)client_netmask & (uint32_t)client_addr); 
        }
        static bool is_ap(IPAddress addr) {
            return ((uint32_t)addr & (uint32_t)ap_netmask) == ((uint32_t)ap_netmask & (uint32_t)ap_addr); 
        }
        static IPAddress forward_ip(IPAddress addr) {
            if(is_ap(addr))
                return client_broadcast();
            if(is_client(addr))
                return ap_broadcast();
            return IPAddress();
        }
        static IPAddress respond_ip(IPAddress addr) {
            if(is_ap(addr))
                return ap_broadcast();
            if(is_client(addr))
                return client_broadcast();
            return IPAddress();
        }
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