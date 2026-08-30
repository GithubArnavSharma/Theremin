#include "quant.hpp"

const float INPUT_SCALE = 0.00784310232847929f;   
const int INPUT_ZERO_POINT = -1;        
const float OUTPUT_SCALE = 0.00390625f;
const int OUTPUT_ZERO_POINT = -128;

int8_t quantize(float value) {
  int8_t x_quantized = value / INPUT_SCALE + INPUT_ZERO_POINT;
  return x_quantized;
}

float dequantize(int8_t value) {
  float q = (value - OUTPUT_ZERO_POINT) * OUTPUT_SCALE;
  return q;
}

double index_to_hz(int index) {
  double cents = 1997.3794084370103 + (index * 20.0);
  double hz = 10.0 * pow(2.0, (cents / 1200.0));
  return hz;
}

