#pragma once
#include <Chirale_TensorFlowLite.h>

#include <Arduino.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "mic_setup.hpp"

#define SAMPLE_RATE 16000
#define BUFFER_SIZE 1024
#define OUTPUT_SIZE 360

extern const tflite::Model* model;
extern tflite::MicroInterpreter* interpreter;
extern TfLiteTensor* input;
extern TfLiteTensor* output;

void setupModel();

void inputModel(int16_t* StreamBuffer);

bool outputModel();

void modelTask(void* param);
