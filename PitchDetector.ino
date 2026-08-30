//Take notes on: Partition Scheme(3 MB app?), quantization, 
//hardware interuppts, diodes, BJT transistors(like pins),
//uint8_t vs int8_t vs int32_t vs float vs double, volatile int, #define 
//ArduTFLite and how all of the commands work
//Tolerance for resistors?

#include <Chirale_TensorFlowLite.h>

#include "model_data.h"
#include <driver/i2s.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#define SAMPLE_RATE 16000
#define BUFFER_SIZE 1024
#define OUTPUT_SIZE 360

#define I2S_WS 5
#define I2S_SD 8
#define I2S_SCK 3
#define SOUND_SAMPLE_RATE    16000
#define SOUND_CHANNEL_COUNT  1
#define I2S_PORT             I2S_NUM_0

int16_t StreamBuffer[BUFFER_SIZE];

i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SOUND_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // or RIGHT, depending on your mic wiring
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false
};

i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
};


const int StreamBufferNumBytes = BUFFER_SIZE * sizeof(int16_t);

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

hw_timer_t* timer = NULL;

//Subjective
constexpr int kTensorArenaSize = 150 * 1024;

alignas(16) uint8_t tensor_arena[kTensorArenaSize];

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


void setup() {
  Serial.begin(115200);
  while(!Serial);

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

  esp_err_t driverInstall = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  esp_err_t setPin = i2s_set_pin(I2S_PORT, &pin_config);

  if (driverInstall != ESP_OK) {
    Serial.println("XXX failed to install I2S");
  }
  if (setPin != ESP_OK) {
    Serial.println("XXX failed to set I2S pins");
  }
  if (i2s_zero_dma_buffer(I2S_PORT) != ESP_OK) {
    Serial.println("XXX failed to zero I2S DMA buffer");
  }
  if (i2s_start(I2S_PORT) != ESP_OK) {
    Serial.println("XXX failed to start I2S");
  }

  Serial.println("... DONE SETUP MIC");

  delay(2000);
}


void loop() {
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_PORT, &StreamBuffer, StreamBufferNumBytes, &bytesRead, portMAX_DELAY);
  
  int16_t minVal = 32767, maxVal = -32768;
  double sum = 0;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    if (StreamBuffer[i] < minVal) minVal = StreamBuffer[i];
    if (StreamBuffer[i] > maxVal) maxVal = StreamBuffer[i];
    sum += StreamBuffer[i];
  }
  Serial.printf("min: %d, max: %d, avg: %ld\n", minVal, maxVal, sum / BUFFER_SIZE);
  if (bytesRead == StreamBufferNumBytes){
    Serial.printf("[");
    for(int i = 0; i < BUFFER_SIZE; ++i) {
      float normalized = StreamBuffer[i] / 2000.0f;
      //Serial.printf("%.7f, ", normalized);
      int8_t x_quantized = quantize(normalized);
      input->data.int8[i] = x_quantized;
    }
    Serial.println("]");
    
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
      Serial.println("Invoke failed!");
      return;
    }

    float highProb = -1;
    int highIndex = -1;
    float sumProb = 0;
    for(int i = 0; i < OUTPUT_SIZE; ++i) {
      int8_t y_quantized = output->data.int8[i];
      float y = dequantize(y_quantized);
      if (y > highProb) {
        highProb = y;
        highIndex = i;
      }
      sumProb += y;
    }

    double freq = index_to_hz(highIndex);

    Serial.printf("Prob: %.3f Index: %d Frequency: %.2f Hz Prob Sum: %.2f\n", highProb, highIndex, freq, sumProb);
    
  } else {
    Serial.printf("Bytes read: %d Stream Buffer Num Bytes: %d\n", bytesRead, StreamBufferNumBytes);
  }
  delay(2000);
}