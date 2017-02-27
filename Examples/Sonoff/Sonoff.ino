#include <mrpc.h>

#define SONOFF_BUTTON 0
#define SONOFF_RELAY  12
#define SONOFF_LED    13

bool light_value = true;

using namespace Json;
using namespace MRPC;

//This service can be called as follows:
//      light()     -> bool
//      light(bool) -> bool

Value light(Value &arg, bool &success) {
    if(arg.isBool()) {
        light_value = arg.asBool();
        digitalWrite(SONOFF_RELAY, light_value);
        digitalWrite(SONOFF_LED, !light_value);    //This LED is active low
    }
    return light_value;     //Return the light value no matter what
}

void setup() {
    Serial.begin(115200);
    init(50123);            //Begin MRPC on UDP port 50123
    pinMode(SONOFF_RELAY, OUTPUT);
    digitalWrite(SONOFF_RELAY, 1);
    pinMode(SONOFF_LED, OUTPUT);
    digitalWrite(SONOFF_LED, 0);
    create_service("light", &light);
}

void loop() {
    poll();
    delay(1);
}