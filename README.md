# MRPC for the ESP8266
MRPC is an RPC framework aimed at mesh networks of embedded devices.
It's designed to be simple, flexible, and light-ish weight.
MRPC uses JSON to meet this goals; the communication protocol requires only that the RPC arguments and return values are JSON values.

This library is a C++ implementation of MRPC for the ESP8266. In addition to the RPC framework, it includes some extra features to make developing applications for the ESP8266 much easier:

## Features
- Simple RPC framework for easy development
- JSON messages for flexibility
- Configurable once deployed
- Storage API for easy persistance
- Allows easy setup in AP mode

## Example
The following is example code for an application to control a built in LED with a 'light' service.

```C
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
}
```

Once uploaded, the ESP8266 will boot into AP mode and begin listening for MRPC messages. It can also be configured to connect to another WiFi access point by browsing to http://192.168.1.1 from a web browser. Messages can be sent from other ESP8266 devices, but let's try sending some from a connected computer with the following Python code:

```python
import mrpc
from mrpc.transport import SocketTransport

MRPC = mrpc.MRPC()
MRPC.use_transport(SocketTransport())
light = MRPC.Proxy("*.light")
toggled_value = not light().get()   #Get the current light value and negate it
light(toggled_value).wait()         #Set the light to the toggled value
```

The Python library is documented more at [its github page](https://github.com/alex-sherman/python-mrpc).
