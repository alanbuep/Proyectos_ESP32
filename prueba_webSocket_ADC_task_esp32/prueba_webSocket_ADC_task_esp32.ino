#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include "config_wifi.h"

const int ledPin = 2;     // GPIO 2 en el ESP32
const int adcPin = 34;    // GPIO 34 para el ADC1

bool adcReadingEnabled = false;
int adcValues[50];
int adcCount = 0;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
// Crea un servidor web en el puerto 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Declaración de funciones
void setupADC();
void taskADCLoop(void *pvParameters);

TaskHandle_t taskADC;  // Handle para la tarea del ADC

void setup() {
  // Inicia el puerto serie para la depuración
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(adcPin, INPUT);

  // Conecta a la red WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a la red WiFi");
  Serial.println(WiFi.status());
  Serial.println(WiFi.localIP());

  // Ruta para obtener el mensaje de bienvenida
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain; charset=utf-8", "¡Hola desde el ESP32!");
  });

  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.println("Cliente conectado");
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.println("Cliente desconectado");
      // Detener la lectura del ADC cuando el cliente se desconecta
      adcReadingEnabled = false;
    } else if (type == WS_EVT_DATA) {
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      if (info->opcode == WS_TEXT) {
        String message = "";
        for (size_t i = 0; i < len; i++) {
          message += (char)data[i];
        }

        // Verificar el contenido del mensaje
        if (message == "on") {
          digitalWrite(ledPin, HIGH);   // Encender el LED
          adcReadingEnabled = true;      // Habilitar la lectura del ADC
        } else if (message == "off") {
          digitalWrite(ledPin, LOW);    // Apagar el LED
          adcReadingEnabled = false;     // Deshabilitar la lectura del ADC
        }
      }
    }
  });

  // Agregar la ruta del WebSocket
  server.addHandler(&ws);

  // Inicia el servidor
  server.begin();

  // Inicia la tarea dedicada al ADC
  setupADC();
}

void loop() {
  // El loop principal puede permanecer vacío ya que la funcionalidad está en las tareas
}

void setupADC() {
  xTaskCreatePinnedToCore(
    taskADCLoop,    // Función que contiene la tarea del ADC
    "TaskADCLoop",  // Nombre de la tarea
    10000,          // Tamaño de la pila de la tarea
    NULL,           // Parámetro de la tarea
    1,              // Prioridad de la tarea
    &taskADC,       // Handle de la tarea
    0);             // Núcleo en el que se ejecutará la tarea (0 o 1)
}

void taskADCLoop(void *pvParameters) {
  while (1) {
    if (adcReadingEnabled) {
      int adcValue = analogRead(adcPin);

      // Utilizar un mutex para evitar conflictos de acceso
      portENTER_CRITICAL(&mux);
      adcValues[adcCount] = adcValue;
      adcCount++;
      portEXIT_CRITICAL(&mux);

      // Si se llena el array, enviarlo a través de WebSocket
      if (adcCount == 0) {
        // Crear un JSON con los valores del ADC
        String json = "{";
        for (int i = 0; i < 100; i++) {
          json += "\"adc" + String(i) + "\":" + String(adcValues[i]);
          if (i < 99) json += ",";
        }
        json += "}";

        // Enviar el JSON a través de WebSocket
        ws.textAll(json.c_str());
      }

      delay(50);  // Ajusta el tiempo de acuerdo a tus necesidades
    }
    delay(10);  // Pequeño retardo para liberar el CPU
  }
}