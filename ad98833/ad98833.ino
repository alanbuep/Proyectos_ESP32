#include <SPI.h>

// PINS para ESP32
#define FSYNC 4     // Puedes elegir cualquier pin disponible en el ESP32
#define DATA_PIN 23 // MOSI (Master Out Slave In)
#define CLOCK_PIN 18 // SCLK (Serial Clock)

// Tipos de onda
#define WAVE_SINE     0x2000
#define WAVE_SQUARE   0x2028
#define WAVE_TRIANGLE 0x2002

// Frecuencia inicial
#define FREQUENCY 2000000

// Variables globales para almacenar la configuración actual
long currentFrequency = FREQUENCY;
int currentWaveform = WAVE_SINE;

void AD9833setup(){
   pinMode(FSYNC, OUTPUT);
   digitalWrite(FSYNC, HIGH);
   SPI.begin(CLOCK_PIN, -1, DATA_PIN); // Configurar el bus SPI
   delay(50); 
   AD9833reset();
}

void AD9833reset(){
   WriteRegister(0x100);
   delay(10);
}

void AD9833setFrequency(long frequency, int Waveform){
   currentFrequency = frequency;
   currentWaveform = Waveform;

   long FreqWord = (frequency * pow(2, 28)) / 25.0E6;
   int MSB = (int)((FreqWord & 0xFFFC000) >> 14);
   int LSB = (int)(FreqWord & 0x3FFF);
   LSB |= 0x4000;
   MSB |= 0x4000; 
   WriteRegister(0x2100);   
   WriteRegister(LSB);
   WriteRegister(MSB);
   WriteRegister(0xC000);
   WriteRegister(Waveform);
}

void WriteRegister(int dat){ 
   SPI.setDataMode(SPI_MODE2);        
   digitalWrite(FSYNC, LOW);
   delayMicroseconds(10); 
   SPI.transfer(dat >> 8);
   SPI.transfer(dat & 0xFF);
   digitalWrite(FSYNC, HIGH);
   SPI.setDataMode(SPI_MODE0);
}

void setup(){
   Serial.begin(9600);
   AD9833setup();
   AD9833setFrequency(currentFrequency, currentWaveform);
   Serial.println("Running on frequency: " + String(currentFrequency));
   Serial.println("Current waveform: " + getWaveformName(currentWaveform));
}

void loop(){
   if (Serial.available() > 0) {
      String command = Serial.readStringUntil('\n');
      processCommand(command);
   }
}

// Función para procesar comandos recibidos por UART
void processCommand(String command) {
   command.trim();
   command.toUpperCase();  // Convierte el comando a mayúsculas

   if (command.startsWith("SET ")) {
      String param = command.substring(4);
      if (isNumber(param)) {
         long freq = param.toInt();
         if (freq > 0) {
            AD9833setFrequency(freq, currentWaveform);
            Serial.println("Frequency set to: " + String(freq));
         } else {
            Serial.println("Invalid frequency");
         }
      } else {
         int wave = parseWaveform(param);
         if (wave != -1) {
            AD9833setFrequency(currentFrequency, wave);
            Serial.println("Waveform set to: " + getWaveformName(wave));
         } else {
            Serial.println("Invalid waveform");
         }
      }
   } else if (command == "GET") {
      Serial.println("Current frequency: " + String(currentFrequency));
      Serial.println("Current waveform: " + getWaveformName(currentWaveform));
   } else {
      Serial.println("Unknown command");
   }
}

// Función para verificar si una cadena es un número
bool isNumber(String str) {
   for (unsigned int i = 0; i < str.length(); i++) {
      if (!isDigit(str.charAt(i))) {
         return false;
      }
   }
   return true;
}

// Función para convertir nombre de la onda a su correspondiente constante
int parseWaveform(String waveString) {
   waveString.toUpperCase();  // Convierte el parámetro a mayúsculas
   if (waveString == "SINE") {
      return WAVE_SINE;
   } else if (waveString == "SQUARE") {
      return WAVE_SQUARE;
   } else if (waveString == "TRIANGLE") {
      return WAVE_TRIANGLE;
   } else {
      return -1; // Error: Tipo de onda desconocido
   }
}

// Función para obtener el nombre de la onda
String getWaveformName(int waveform) {
   switch (waveform) {
      case WAVE_SINE:
         return "SINE";
      case WAVE_SQUARE:
         return "SQUARE";
      case WAVE_TRIANGLE:
         return "TRIANGLE";
      default:
         return "UNKNOWN";
   }
}
