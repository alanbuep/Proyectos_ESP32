#include "esp_system.h"
#include "rom/ets_sys.h"
#include <driver/i2s.h>
#include "SPI.h"
#include "setupI2S.h"

#define LED_BUILTIN 2
#define TIME_TIMER (I2S_DMA_BUF_LEN * 1000000 / I2S_SAMPLE_RATE)

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
  timerAlarmWrite(timer, TIME_TIMER, true);               // Interrupción cada 1 segundo
}

void loop() {
  if (has_expired) {
    i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
    for (int i = 0; i < I2S_DMA_BUF_LEN; i++) {
      Serial.println(buffer[i] & 0x0FFF);
    }
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    has_expired = false;
  }

  if (Serial.available()) {
    String str = Serial.readStringUntil('\n');
    str.trim();
    if (str == "off") {
      timerAlarmDisable(timer);
      Serial.println("El temporizador ha sido desactivado");
    }
    if (str == "on") {
      Serial.println("Timer iniciado - Medición Iniciada");
      timerAlarmEnable(timer);
    }
  }
}