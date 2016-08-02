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