#include "esp_system.h"
#include "rom/ets_sys.h"
#include <driver/i2s.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "setupI2S.h"

#define LED_BUILTIN 2
// #define TIME_TIMER (I2S_DMA_BUF_LEN * 1000000 / I2S_SAMPLE_RATE)
#define TIME_TIMER (16000)
// #define TIME_TIMER (1000000 / I2S_SAMPLE_RATE)

File file;

hw_timer_t* timer = NULL;
volatile bool has_expired = false;
volatile uint16_t i2s_value = 0;

void IRAM_ATTR timerInterrupcion() {
  has_expired = true;
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
}

void loop() {
  if (has_expired) {
    i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
    for (int i = 0; i < I2S_DMA_BUF_LEN; i++) {
      file.println(buffer[i] & 0x0FFF);
    }
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    has_expired = false;
  }

  if (Serial.available()) {
    String str = Serial.readStringUntil('\n');
    str.trim();
    if (str == "off") {
      file.close();
      timerAlarmDisable(timer);
      Serial.println("El temporizador ha sido desactivado");
      Serial.println("Data saved!!!");
    }
    if (str == "on") {
      // Abre el archivo para escribir
      file = SD.open("/data.txt", FILE_WRITE);
      if (!file) {
        Serial.println("Failed to open file for writing");
        return;
      }
      Serial.println("Timer iniciado - Medición Iniciada");
      timerAlarmEnable(timer);
    }
  }
}
