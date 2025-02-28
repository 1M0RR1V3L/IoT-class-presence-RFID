#ifndef SEND_DATA_H
#define SEND_DATA_H

#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_SSD1306.h>
#include <map>
#include <set>
#include <unordered_map>

String base_url = "https://api.tago.io/data";
String TAGOIO_DEVICE_TOKEN = "2a91041c-8746-45c8-bd3a-b912cc8c9b54";  // Use seu token

std::map<String, String> studentData = {
    {"8545ABCD", "Antônio Lucas"},
    {"12345678", "Matheus Rabelo"},
    {"AAB34211", "Robert Michael"},
    {"ABCDABCD", "José Eric"}
};

std::set<String> sentIDs;  // Guarda IDs já enviados

void sendTagoIoData(String student_id, Adafruit_SSD1306 &display, int GREEN_LED2, int RED_LED2) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(base_url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Device-Token", TAGOIO_DEVICE_TOKEN);

    DynamicJsonDocument doc(512); 
    JsonArray data = doc.to<JsonArray>();

    if (studentData.find(student_id) != studentData.end()) {
      String student_name = studentData[student_id];
      String status = (sentIDs.find(student_id) != sentIDs.end()) ? "out" : "in";

      if (status == "out") {
        sentIDs.erase(student_id);
      } else {
        sentIDs.insert(student_id);
      }

      JsonObject obj1 = data.createNestedObject();
      obj1["variable"] = "student_id";
      obj1["value"] = student_id;

      JsonObject obj2 = data.createNestedObject();
      obj2["variable"] = "name";
      obj2["value"] = student_name;

      JsonObject obj3 = data.createNestedObject();
      obj3["variable"] = "status";
      obj3["value"] = status;
      
    } else {
      Serial.println("ID não cadastrado!");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.println("ID não cadastrado!");
      display.display();
      return;
    }

    String payload;
    serializeJson(data, payload);
    Serial.println("Enviando JSON: " + payload); 

    int responseCode = http.POST(payload);
    Serial.print("HTTP Response Code: ");
    Serial.println(responseCode);

    String response;
    if (responseCode > 0) {
      response = http.getString();
      Serial.println("Resposta do Servidor: " + response);
    }

    http.end();

    // Lógica de exibição
    if (responseCode == 200 || responseCode == 202) {
      digitalWrite(GREEN_LED2, HIGH);
      digitalWrite(RED_LED2, LOW);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.println("ID Enviado com Sucesso!");
      display.display();
    } else {
      digitalWrite(RED_LED2, HIGH);
      digitalWrite(GREEN_LED2, LOW);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.println("Erro ao Enviar!");
      display.display();
    }
  } else {
    Serial.println("Not connected to WiFi");
  }
}



#endif