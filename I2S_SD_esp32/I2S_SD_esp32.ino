#include "esp_system.h"
#include "rom/ets_sys.h"
#include <driver/i2s.h>
#include "FS.h"
#include "SD.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "SPI.h"
#include "setupI2S.h"

#define LED_BUILTIN 2
#define TIME_TIMER (1000)    // Ajustado para 1000 Hz
#define BUFFER_SIZE (10000)  // Tamaño del buffer

File file;

TaskHandle_t Task1;
TaskHandle_t Task2;

volatile uint16_t buffer_index = 0;

hw_timer_t* timer = NULL;
volatile bool has_expired = false;
volatile uint16_t i2s_value = 0;
uint16_t big_buffer[BUFFER_SIZE];
uint16_t big_buffer_index = 0;

void IRAM_ATTR timerInterrupcion() {
  has_expired = true;
}

void readData(void* parameter) {
  while (1) {
    // Lee los datos del I2S
    i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
    for (int i = 0; i < I2S_DMA_BUF_LEN; i++) {
      // Almacena los datos en el buffer
      big_buffer[big_buffer_index++] = buffer[i] & 0x0FFF;
      if (big_buffer_index >= BUFFER_SIZE) {
        // Si el buffer está lleno, notifica a la tarea writeData
        big_buffer_index = 0;
        xTaskNotifyGive(Task2);
      }
    }
  }
}

void writeData(void* parameter) {
  while (1) {
    // Espera a ser notificado por la tarea readData
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
      // Abre el archivo para escribir
      
      // Escribe los datos del buffer en la tarjeta SD
      for (int i = 0; i < BUFFER_SIZE; i++) {
        file.println(big_buffer[i]);
      }
      file.flush();
      Serial.println("Termino la medicion");
    }
  }
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  i2sInit();

  timer = timerBegin(0, 80, true);                        // Timer 0, divisor de reloj 80
  timerAttachInterrupt(timer, &timerInterrupcion, true);  // Adjuntar la función de manejo de interrupción
  timerAlarmWrite(timer, TIME_TIMER, true);

  Serial.begin(115200);
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

  file = SD.open("/data.txt", FILE_WRITE);
      if (!file) {
        Serial.println("Failed to open file for writing");
        return;
      }

  xTaskCreatePinnedToCore(readData, "ReadData", 10000, NULL, 1, &Task1, 0);
  xTaskCreatePinnedToCore(writeData, "WriteData", 10000, NULL, 1, &Task2, 1);
}

void loop() {
}