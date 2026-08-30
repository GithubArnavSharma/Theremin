#pragma once
#include <driver/i2s.h>
#include <Arduino.h>

#define SAMPLE_RATE 16000
#define BUFFER_SIZE 1024
#define OUTPUT_SIZE 360

#define I2S_WS 5
#define I2S_SD 8
#define I2S_SCK 3
#define SOUND_SAMPLE_RATE    16000
#define SOUND_CHANNEL_COUNT  1
#define I2S_PORT             I2S_NUM_0

extern i2s_config_t i2s_config;
extern i2s_pin_config_t pin_config;

extern const int StreamBufferNumBytes;

extern int16_t StreamBuffer[BUFFER_SIZE];

extern QueueHandle_t micQueue;

void installAndSetPin();

bool readMic();

void micTask(void* param);
