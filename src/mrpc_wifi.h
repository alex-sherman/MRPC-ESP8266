#ifndef _MRPC_MESH_H_
#define _MRPC_MESH_H_
#include "mrpc.h"
#include <ESP8266WiFi.h>

namespace MRPC {
    class MRPCWifi {
    public:
        void connect(Json::Object &wifi_settings);
        void setupWiFiAP(const char* password);
    };
}
#endif