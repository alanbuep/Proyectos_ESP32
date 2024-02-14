#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2s.h"

#define LED_GPIO GPIO_NUM_2
#define I2S_DMA_BUF_LEN 8
#define I2S_SAMPLE_RATE 3000
#define ADC_INPUT ADC1_CHANNEL_4

// Variables globales
size_t bytes_read;
uint16_t buffer[I2S_DMA_BUF_LEN] = {0};

void i2sInit() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate =  I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = I2S_DMA_BUF_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_adc_mode(ADC_UNIT_1, ADC_INPUT);
  i2s_adc_enable(I2S_NUM_0);
  adc1_config_channel_atten(ADC_INPUT, ADC_ATTEN_DB_11);
}

void blinky(void *pvParameter) {
  gpio_pad_select_gpio(LED_GPIO);
  gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
  while (1) {
    gpio_set_level(LED_GPIO, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(LED_GPIO, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void uart_task(void *pvParameter) {


  while (1) {
    i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, 0);
    Serial.print(buffer[0] & 0x0FFF);
    Serial.print(" ");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  i2sInit();
  xTaskCreate(&blinky, "blinky", 1024, NULL, 5, NULL);
  xTaskCreate(&uart_task, "uart_task", 1024, NULL, 5, NULL);
}

void loop() {
  // Deja esta función vacía si no necesitas hacer nada aquí.
}
