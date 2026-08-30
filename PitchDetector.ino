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
