#include <Wire.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include "sendData.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6
#define WIFI_TIMEOUT 30000

// LEDs de Conexão
#define RED_LED 26     // LED Vermelho (Esquerda) - Conexão WiFi
#define GREEN_LED 27   // LED Verde (Esquerda) - Conexão WiFi

// LEDs de Status de Requisição
#define RED_LED2 5     // LED Vermelho (Direita) - Erro na Requisição
#define GREEN_LED2 17  // LED Verde (Direita) - Sucesso na Requisição

// Configuração do Keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 14, 23}; // Linhas R1-R4
byte colPins[COLS] = {22, 21, 16, 15}; // Colunas C1-C4
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String inputCode = "";
String lastChar = ""; // Armazena o último caractere pressionado para remoção

void signalLED(int ledId) {
  digitalWrite(ledId, HIGH);
  delay(1500);
  digitalWrite(ledId, LOW);
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Connecting to WiFi...");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');  
    digitalWrite(RED_LED, LOW);  
    digitalWrite(GREEN_LED, HIGH); 
    delay(1000);
    digitalWrite(GREEN_LED, LOW);
  }
  Serial.print("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(RED_LED, HIGH);  
  digitalWrite(GREEN_LED, LOW);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Connected to WiFi");
  display.println("IP:");
  display.println(WiFi.localIP());
  display.display();
  delay(2000);
}

void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED2, OUTPUT);
  pinMode(RED_LED2, OUTPUT);

  Wire.begin(22, 21); 
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED display failed to start"));
    while (true);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  connectToWiFi();
  Serial.println("\nDigite um ID NFC (simulado) e pressione Enter:");
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Digite o ID NFC:");
  display.display();
}

void loop() {
  // Leitura via Keypad
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {  // Se pressionado '#', envia dados e limpa a entrada
      if (!inputCode.isEmpty()) {
        Serial.println("sending...");
        sendTagoIoData(inputCode, display, GREEN_LED2, RED_LED2); 
        inputCode = ""; // Limpa a entrada após enviar
        lastChar = "";  // Limpa o último caractere
      }
    } else if (key == '*') {  // Se pressionado '', remove o último caractere
      if (!inputCode.isEmpty()) {
        Serial.print(lastChar);  
        Serial.println(" removed");
        inputCode.remove(inputCode.length() - 1);  // Remove o último caractere
        lastChar = inputCode.length() > 0 ? inputCode.substring(inputCode.length() - 1) : ""; // Atualiza o último caractere
      }
    } else {
      lastChar = String(key);  // Armazena o último caractere pressionado
      inputCode += key;  // Adiciona o caractere à entrada
    }

    // Atualiza OLED a cada tecla pressionada
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.println("Digitando:");
    display.println(inputCode);
    display.display();
    
    Serial.print(key);
    Serial.println(" pressed");
  }
}