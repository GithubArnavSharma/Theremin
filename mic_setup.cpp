#include "mic_setup.hpp" 

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


void installAndSetPin() {
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
}

bool readMic() {
    size_t bytesRead = 0;
    esp_err_t result = i2s_read(I2S_PORT, &StreamBuffer, StreamBufferNumBytes, &bytesRead, portMAX_DELAY);
    return bytesRead == StreamBufferNumBytes;
}