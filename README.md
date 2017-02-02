# MRPC for the ESP8266
A library for the ESP8266 to facilitate simpler, and efficient development of wireless applications.
MRPC has libraries for several languages and platforms including the ESP8266 (C++), Android (Java), and a general Python library to support communication between a wide variety of devices.

This library is a C++ implementation of MRPC for the ESP8266. In addition to the RPC framework, it includes some extra features to make developing applications for the ESP8266 much easier:

## Features
- Simple RPC framework for easy development
- JSON messages for flexibility
- Configurable once deployed
- Storage API for easy persistance
- Allows easy setup in AP mode

# General Overview

The core idea behind MRPC, is remote procedure call. Any functionality we want to implement on the ESP, we will just write a function for and be able to call remotely as we wish. These functions are referred to as a [service](/alex-sherman/MRPC-ESP8266/wiki/Service) in MRPC.

![](https://docs.google.com/drawings/d/16HAHWeJ8-HilS22Vmc00pYQiUevSUPRaiXSvHlPPUM8/pub?w=613&h=447)

# How to Use MRPC

Developing with MRPC on the ESP is meant to be easy. Follow these steps to get your WiFi application up and running:

1. Download and install the MRPC and Embedded-Json libraries into your Arduino IDE
2. `#include <mrpc.h>`
3. Write your service matching the ServiceMethod signature
4. Add `MRPC::init(50123)` to your `setup()`
5. Register your service with `create_service(<name>, <service method>)`
6. Make sure to call `MRPC::poll()` in your `loop()`

After compiling and uploading to an ESP, you're ready to go! If you can connect over serial to the ESP you can already start invoking services. To configure the ESP WiFi settings to connect to a network you can run the built in `wifi` service like so: `wifi(["ssid", "password"])`. Otherwise the ESP will automatically bring up an access point with an SSID like `MRPC XXXX`. You can connect to this network and browse to `http://192.168.1.1` and use the web form to configure WiFi settings.

# Example Usage

Let's quickly write an application on the ESP that allows us to turn on and off its builtin LED.

```C++
#include <mrpc.h>

bool light_value = true;

using namespace Json;
using namespace MRPC;

//This service can be called as follows:
//      light()     -> bool
//      light(bool) -> bool

Value light(Service *self, Value &arg, bool &success) {
    if(arg.isBool()) {
        light_value = arg.asBool();
        digitalWrite(LED_BUILTIN, !light_value);    //This LED is active low
    }
    return light_value;     //Return the light value no matter what
}

void setup() {
    Serial.begin(115200);
    init(50123);            //Begin MRPC on UDP port 50123
    pinMode(LED_BUILTIN, OUTPUT);
    create_service("light", &light);
}

void loop() {
    poll();
    delay(10);              //Sleep a little while to not spin constantly
}
```


Each MRPC node in a network has a set of **aliases** associated with it, which allow devices to call services on a specific MRPC node. The combination of an alias and service name is called a **path**.

<div>Icons made by <a href="http://www.flaticon.com/authors/madebyoliver" title="Madebyoliver">Madebyoliver</a>, <a href="http://www.flaticon.com/authors/vectors-market" title="Vectors Market">Vectors Market</a>, <a href="http://www.flaticon.com/authors/epiccoders" title="EpicCoders">EpicCoders</a> from <a href="http://www.flaticon.com" title="Flaticon">www.flaticon.com</a> is licensed by <a href="http://creativecommons.org/licenses/by/3.0/" title="Creative Commons BY 3.0" target="_blank">CC 3.0 BY</a></div>
