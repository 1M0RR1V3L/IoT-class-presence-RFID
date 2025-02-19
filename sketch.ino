#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6
#define WIFI_TIMEOUT 30000

#define RED_LED 5
#define GREEN_LED 17
#define GREEN_LED2 27

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

DynamicJsonDocument doc(128);
String base_url = "https://api.tago.io/data";
String TAGOIO_DEVICE_TOKEN = "2a91041c-8746-45c8-bd3a-b912cc8c9b54";
bool useLed = true;
String inputCode = "";  

void signalLED(int ledId) {
  if (useLed) {
    digitalWrite(ledId, HIGH);
    delay(1500);
    digitalWrite(ledId, LOW);
  }
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao WiFi: ");
  Serial.print(WIFI_SSID);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("\nWiFi Conectado! IP: " + WiFi.localIP().toString());
}

void sendTagoIoData(String student_id) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(base_url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Device-Token", TAGOIO_DEVICE_TOKEN);

    doc["variable"] = "student_id";
    doc["value"] = student_id;

    String payload;
    serializeJson(doc, payload);

    int responseCode = http.POST(payload);
    Serial.print("Código HTTP: ");
    Serial.println(responseCode);

    if (responseCode == 202) {
      signalLED(GREEN_LED);
    } else {
      signalLED(RED_LED);
    }

    if (responseCode > 0) {
      Serial.println("Resposta do servidor: " + http.getString());
    }
    http.end();
  } else {
    Serial.println("WiFi não conectado!");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED2, OUTPUT);

  connectToWiFi();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erro ao inicializar OLED!");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 20);
  display.println("Pronto!");
  display.display();

  Serial.println("Digite um ID NFC:");
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    Serial.print("Tecla pressionada: ");
    Serial.println(key);

    if (key == '#') { 
      if (!inputCode.isEmpty()) {
        Serial.println("Enviando para o TagoIO: " + inputCode);
        sendTagoIoData(inputCode);

        display.clearDisplay();
        display.setCursor(10, 20);
        display.println("Enviado:");
        display.println(inputCode);
        display.display();

        inputCode = ""; 
      }
    } else if (key == '*') {  
      if (!inputCode.isEmpty()) {
        inputCode.remove(inputCode.length() - 1);
      }
    } else {
      inputCode += key; 
    }

    display.clearDisplay();
    display.setCursor(10, 20);
    display.println("Digitando:");
    display.println(inputCode);
    display.display();
  }
}
