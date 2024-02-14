#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include "config_wifi.h"

const int ledPin = 2;     // GPIO 2 en el ESP32
const int adcPin = 34;    // GPIO 34 para el ADC1

bool adcReadingEnabled = false;

// Crea un servidor web en el puerto 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

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
}

void loop() {
  if (adcReadingEnabled) {
    int adcValue = analogRead(adcPin);

    // Envía el valor del ADC a través de WebSocket
    String json = "{\"adc\": " + String(adcValue) + "}";
    ws.textAll(json.c_str());

    delay(1000);
  }
}