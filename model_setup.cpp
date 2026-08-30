#include "model_setup.hpp"
#include "quant.hpp"

#include "model_data.h"

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

constexpr int kTensorArenaSize = 150 * 1024;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];

void setupModel() {
    model = tflite::GetModel(crepe_small_int8_tflite);

    if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.println("Model provided and schema version are not equal!");
        return;
    }

    static tflite::AllOpsResolver resolver;

    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        Serial.println("AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

}

void inputModel(int16_t* StreamBuffer) {
    int16_t minVal = 32767, maxVal = -32768;
    double sum = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (StreamBuffer[i] < minVal) minVal = StreamBuffer[i];
        if (StreamBuffer[i] > maxVal) maxVal = StreamBuffer[i];
        sum += StreamBuffer[i];
    }
    Serial.printf("min: %d, max: %d, avg: %ld\n", minVal, maxVal, sum / BUFFER_SIZE);
    Serial.printf("[");
    for(int i = 0; i < BUFFER_SIZE; ++i) {
        float normalized = StreamBuffer[i] / 2000.0f;
        //Serial.printf("%.7f, ", normalized);
        int8_t x_quantized = quantize(normalized);
        input->data.int8[i] = x_quantized;
    }
    Serial.println("]");
}

bool outputModel() {
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
      return false;
    }

    float highProb = -1;
    int highIndex = -1;
    for(int i = 0; i < OUTPUT_SIZE; ++i) {
      int8_t y_quantized = output->data.int8[i];
      float y = dequantize(y_quantized);
      if (y > highProb) {
        highProb = y;
        highIndex = i;
      }
    }

    double freq = index_to_hz(highIndex);

    Serial.printf("Prob: %.3f Index: %d Frequency: %.2f Hz", highProb, highIndex, freq);

    return true;
}
