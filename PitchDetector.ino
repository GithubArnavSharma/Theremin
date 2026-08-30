//Take notes on: Partition Scheme(3 MB app?), quantization, 
//hardware interuppts, diodes, BJT transistors(like pins),
//uint8_t vs int8_t vs int32_t vs float vs double, volatile int, #define 
//ArduTFLite and how all of the commands work
//Tolerance for resistors?

#include "mic_setup.hpp" 
#include "model_setup.hpp"
#include "quant.hpp" 

void setup() {
  Serial.begin(115200);
  while(!Serial);

  setupModel();
  installAndSetPin();

  delay(2000);
}


void loop() {
  if (readMic()) {
    inputModel(StreamBuffer);
    if (!outputModel()) {
      Serial.println("Invoke failed!");
      return;
    }
  }
  delay(2000);
}