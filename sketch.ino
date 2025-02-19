#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6
#define WIFI_TIMEOUT 30000

#define RED_LED 5
#define GREEN_LED 17
#define GREEN_LED2 27


DynamicJsonDocument doc(128);
String payload = "";
String base_url = "https://api.tago.io/data";
String TAGOIO_DEVICE_TOKEN = "2a91041c-8746-45c8-bd3a-b912cc8c9b54";
bool useLed = true;

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
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.print("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());
}

void sendTagoIoData(String student_id){
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(base_url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Device-Token", TAGOIO_DEVICE_TOKEN);  // Autenticação no TagoIO

    doc["variable"] = "student_id";
    doc["value"] = student_id;

    String payload;
    serializeJson(doc, payload);

    int responseCode = http.POST(payload);
    Serial.print("HTTP Response Code: ");
    Serial.println(responseCode);

    if (responseCode == 202) {
      signalLED(GREEN_LED);
    } else {
      signalLED(RED_LED);
    }

    String response;
    if (responseCode > 0) {
      response = http.getString();
      Serial.println("Resposta do Servidor: " + response);
    }
    http.end();
  } else {
    Serial.println("Not connected to WiFi");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  connectToWiFi();
  Serial.println("\nDigite um ID NFC (simulado) e pressione Enter:");
}

void loop() {
  if (Serial.available() > 0) {
    pinMode(GREEN_LED2, HIGH);
    String student_id = Serial.readStringUntil('\n');
    student_id.trim(); 

    if (student_id.length() > 0) {
      Serial.print("Enviando ID NFC para o TagoIO: ");
      Serial.println(student_id);
      sendTagoIoData(student_id);
    }
    Serial.println("\nDigite outro ID NFC ou pressione Enter para reenviar:");
  }
}