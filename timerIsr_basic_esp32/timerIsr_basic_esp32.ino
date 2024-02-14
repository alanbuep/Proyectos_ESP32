#include "esp_system.h"
#include "rom/ets_sys.h"

#define LED_BUILTIN 2

hw_timer_t* timer = NULL;
volatile bool has_expired = false;

void IRAM_ATTR timerInterrupcion() {
  has_expired = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  timer = timerBegin(0, 80, true);                        // Timer 0, divisor de reloj 80
  timerAttachInterrupt(timer, &timerInterrupcion, true);  // Adjuntar la función de manejo de interrupción
  timerAlarmWrite(timer, 1000000, true);                  // Interrupción cada 1 segundo
}

void loop() {
  if (has_expired) {
    // Tareas a realizar cuando se activa la interrupción del Timer
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));  // Alternar el estado del LED
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
      timerAlarmEnable(timer);
      Serial.println("Timer iniciado");
    }
  }
}