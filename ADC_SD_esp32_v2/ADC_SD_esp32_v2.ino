#include "esp_system.h"
#include "rom/ets_sys.h"
#include <driver/i2s.h>
#include "FS.h"
#include "SD.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "SPI.h"

File file;

#define BUFFER_SIZE 1024  // Tamaño del buffer
#define LED_BUILTIN 2

uint16_t buffer1_1[BUFFER_SIZE];
uint16_t buffer1_2[BUFFER_SIZE];
uint16_t buffer2_1[BUFFER_SIZE];
uint16_t buffer2_2[BUFFER_SIZE];

volatile int buffer1_index = 0;
volatile int buffer2_index = 0;

volatile uint16_t adc_value = 0;

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

volatile bool useBuffer1 = true;

void readData(void* parameter) {
  while (1) {
    adc_value = analogRead(32);  // Lee el valor del ADC en el pin 32
    if (useBuffer1) {
      buffer1_1[buffer1_index] = adc_value;
    } else {
      buffer1_2[buffer1_index] = adc_value;
    }
    buffer1_index++;

    adc_value = analogRead(33);  // Lee el valor del ADC en el pin 33
    if (useBuffer1) {
      buffer2_1[buffer2_index] = adc_value;
    } else {
      buffer2_2[buffer2_index] = adc_value;
    }
    buffer2_index++;

    if (buffer1_index >= BUFFER_SIZE && buffer2_index >= BUFFER_SIZE) {
      buffer1_index = 0;
      buffer2_index = 0;
      useBuffer1 = !useBuffer1;  // Cambia el buffer que se está utilizando
      xTaskNotifyGive(Task3);
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void writeData(void* parameter) {
  while (1) {
    // Espera a ser notificado por las tareas readData
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
      // Escribe los datos del buffer1 en la tarjeta SD
      if (!useBuffer1) {
        for (int i = 0; i < BUFFER_SIZE; i++) {
          file.println(String(buffer1_1[i]) + ";" + String(buffer2_1[i]));
        }
      } else {
        for (int i = 0; i < BUFFER_SIZE; i++) {
          file.println(String(buffer1_2[i]) + ";" + String(buffer2_2[i]));
        }
      }
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
    file.flush();
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
}

void controller(void* parameter) {
  while (1) {
    if (Serial.available()) {
      String str = Serial.readStringUntil('\n');
      str.trim();
      if (str == "off") {
        vTaskDelete(Task2);
        vTaskDelete(Task3);
        Serial.println("Medición Finalizada");
      }
      if (str == "on") {
        Serial.println("Medición Iniciada");
        xTaskCreatePinnedToCore(readData, "ReadData", 10000, NULL, 1, &Task2, 0);
        xTaskCreatePinnedToCore(writeData, "WriteData", 10000, NULL, 1, &Task3, 1);
      }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

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

  xTaskCreatePinnedToCore(controller, "controller", 10000, NULL, 1, &Task1, 1);
}

void loop() {
}