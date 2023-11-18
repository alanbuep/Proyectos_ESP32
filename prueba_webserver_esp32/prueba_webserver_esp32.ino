#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "config_wifi.h"

// Crea un servidor web en el puerto 80
AsyncWebServer server(80);

void setup() {
  // Inicia el puerto serie para la depuración
  Serial.begin(115200);

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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain; charset=utf-8", "¡Hola desde el ESP32!");
  });

  // Inicia el servidor
  server.begin();
}

void loop(){}
