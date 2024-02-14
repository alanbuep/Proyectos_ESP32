#pragma once

// Definiciones para la configuración de I2S
#define I2S_SAMPLE_RATE (1000)
#define ADC_INPUT (ADC1_CHANNEL_4)  // Pin 32
#define I2S_DMA_BUF_LEN (1000)

// Variables globales
size_t bytes_read;
uint16_t buffer[I2S_DMA_BUF_LEN] = { 0 };

// Inicialización de I2S
void i2sInit() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = I2S_DMA_BUF_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_adc_mode(ADC_UNIT_1, ADC_INPUT);
  i2s_adc_enable(I2S_NUM_0);
  adc1_config_channel_atten(ADC_INPUT, ADC_ATTEN_DB_0);
  // adc1_config_channel_atten(ADC_INPUT, ADC_ATTEN_DB_2_5);
  // adc1_config_channel_atten(ADC_INPUT, ADC_ATTEN_DB_6);
  // adc1_config_channel_atten(ADC_INPUT, ADC_ATTEN_DB_11);
}