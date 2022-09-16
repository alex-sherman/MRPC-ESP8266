#include <mrpc.h>
#include "ESP32TimerInterrupt.h"

using namespace Json;
using namespace MRPC;

#define txPin 12

// In time slices
int preambleLen = 12 * 2;
int preamblePause = 6 * 2;
int finalPause = 40;
// In bits
int messageLen = 66;
char message[] = { 0xE4, 0x2A, 0xF5, 0x6D, 0x2E, 0x88, 0x07, 0x7F, 0x80 };
int messageInterval = 400;

int messageIndex = 0;
int repeatsIndex = -1;
int repeats = 4;

bool radioValue(int index, bool& value) {
  if (index < preambleLen) {
    value = (index % 2) == 0;
    return true;
  }
  index -= preambleLen;
  if (index < preamblePause) {
    value = false;
    return true;
  }
  index -= preamblePause;
  if (index < messageLen * 3) {
    int r = index % 3;
    int b = (index / 3) % 8;
    char c = message[index / 24];
    value = r == 0 ? true : (r == 1 ? c & (1 << (7 - b)) : false);
    return true;
  }
  value = false;
  index -= messageLen * 3;
  return index < finalPause;
}

ESP32Timer timer(0);
bool IRAM_ATTR timerCallback(void * timerNo)
{
  bool radioOutput = false;
  bool continueTimer = radioValue(messageIndex, radioOutput);
  messageIndex++;
  digitalWrite(txPin, radioOutput);
  if (!continueTimer) {
    messageIndex = 0;
    repeatsIndex++;
    if (repeatsIndex == repeats) {
      repeatsIndex = -1;
      timer.disableTimer();
    }
  }
  return true;
}

Value light(Value &arg, bool &success) {
    if (repeatsIndex != -1) return false;
    repeatsIndex = 0;
    timer.setInterval(messageInterval, timerCallback);
    return true;
}

void setup() {
    Serial.begin(115200);
    pinMode(txPin, OUTPUT);
    
    init(50123);            //Begin MRPC on UDP port 50123
    create_service("light", &light);
}

void loop() {
    poll();
    delay(1);
}
