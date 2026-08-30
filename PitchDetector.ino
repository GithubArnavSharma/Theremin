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

  xTaskCreate(micTask, "micTask", 4096, NULL, 1, NULL);
  xTaskCreate(modelTask, "modelTask", 8192, NULL, 2, NULL);
}


void loop() {

}
