#include <driver/i2s.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "setupI2S.h"
#include "setupSD.h"

File file;

void setup() {
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

  i2sInit();

  // Abre el archivo para escribir
  file = SD.open("/data.txt", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
}

void loop() {
  unsigned long startMicros = ESP.getCycleCount();
  i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, 0);
  unsigned long stopMicros = ESP.getCycleCount();

  // Escribe los datos en el archivo
  file.println(buffer[0] & 0x0FFF);

  // Cierra el archivo después de 30 segundos
  static unsigned long timer = 0;
  if (millis() - timer > 30000) {
    timer = millis();
    file.close();
    Serial.println("Data saved!");
  }
}