#pragma once

#include <Arduino.h>

extern const float INPUT_SCALE;
extern const int INPUT_ZERO_POINT;      
extern const float OUTPUT_SCALE;
extern const int OUTPUT_ZERO_POINT;

int8_t quantize(float value);

float dequantize(int8_t value);

double index_to_hz(int index);
