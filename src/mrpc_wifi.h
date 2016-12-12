#ifndef _MRPC_MESH_H_
#define _MRPC_MESH_H_
#include "mrpc.h"
#include <ESP8266WiFi.h>

namespace MRPC {
    class MRPCWifi {
    public:
        void connect(Json::Object &wifi_settings);
        void setupWiFiAP(const char* password);
        static IPAddress client_addr;
        static IPAddress client_netmask;
        static IPAddress ap_addr;
        static IPAddress ap_netmask;
        static bool is_client(IPAddress addr) {
            return ((uint32_t)addr & (uint32_t)client_netmask) == ((uint32_t)client_netmask & (uint32_t)client_addr); 
        }
        static bool is_ap(IPAddress addr) {
            return ((uint32_t)addr & (uint32_t)ap_netmask) == ((uint32_t)ap_netmask & (uint32_t)ap_addr); 
        }
    };
}
#endif