
#include <IRremoteESP8266.h>
#include <mrpc.h>
#include <FS.h>

using namespace Json;

IRrecv irrecv(D1);
IRsend irsend(D5);

decode_results results;
Object *key_codes;
Object &get_key_codes() {
  if(key_codes == NULL) {
    File f = SPIFFS.open("/key_codes.json", "r");
    size_t size = f.size();
    std::unique_ptr<char[]> buf(new char[size]);
    f.readBytes(buf.get(), size);
    Value v = parse(buf.get());
    if(v.isObject()) {
      key_codes = &v.asObject();
    }
    else {
      v.free_parsed();
      key_codes = new Object();
    }
    f.close();
  }
  return *key_codes;
}

void save_key_codes() {
  Object &key_codes = get_key_codes();
  //size_t size = measure(key_codes);
  //std::unique_ptr<char[]> buf(new char[size]);
  //dump(key_codes, buf.get(), size);
  File f = SPIFFS.open("/key_codes.json", "w");
  print(key_codes, f);
  f.close();
}

Value _irsend(Value &arg, bool &success) {
  if(arg.isInt()) {
    irsend.sendNEC(arg.asInt(), 32);
    return true;
  }
  else if(arg.isString()) {
    const char* s = arg.asString();
    if(get_key_codes().has(s)) {
      irsend.sendNEC(get_key_codes()[s].asInt(), 32);
      return true;
    }
    return false;
  }
}

Value program(Value &arg, bool &success) {
  if(arg.isArray()) {
    Json::Array &args = arg.asArray();
    if(!args[0].isString() && args[1].isInt()) return false;
    get_key_codes()[args[0].asString()] = args[1].asInt();
    save_key_codes();
    return true;
  }
  else if(arg.isString()) {
    irrecv.resume();
    for(int i = 0; i < 5; i++) {
      if (irrecv.decode(&results)) {
        get_key_codes()[arg.asString()] = (int)results.value;
        irrecv.resume(); // Receive the next value
        save_key_codes();
        return true;
      }
      delay(1000);
    }
  }
  return get_key_codes().clone();
}

void setup()
{
  pinMode(D3, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D1, INPUT);
  digitalWrite(D3, 1);
  digitalWrite(D2, 0);
  digitalWrite(D7, 0);
  Serial.begin(115200);
  irrecv.enableIRIn(); // Start the receiver
  irrecv.resume();
  irsend.begin();
  SPIFFS.begin();
  MRPC::init(50123);
  MRPC::create_service("irsend", &_irsend);
  MRPC::create_service("program", &program);
  Serial.print("Key codes: ");
  println(get_key_codes(), Serial);
}

void loop() {
  MRPC::poll();
  delay(10);
}
