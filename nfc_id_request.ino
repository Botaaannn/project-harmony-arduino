#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>


#define SS_PIN 5
#define RST_PIN 2
MFRC522 mfrc522(SS_PIN, RST_PIN);


const char* ssid = "";
const char* password = "";
const char* apiBase = "https://project-qol-backend.onrender.com/api/student?id=";  // change to your real API


void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS
  mfrc522.PCD_Init();
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi failed to connect. Continue offline.");
  }
  Serial.println("Ready. Present a card...");
}


void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;


  String uidHex = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uidHex += "0";
    uidHex += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidHex.toUpperCase();
  Serial.print("Card UID: ");
  Serial.println(uidHex);


  if (WiFi.status() == WL_CONNECTED) {
    String url = String(apiBase) + uidHex;
    Serial.print("Requesting URL: ");
    Serial.println(url);


    HTTPClient http;
    http.begin(url); // for HTTPS you may need fingerprints/certs
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.print("HTTP code: ");
      Serial.println(httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(F("API response:"));
        Serial.println(payload); // JSON string from API
      }
    } else {
      Serial.print("HTTP request failed, error: ");
      Serial.println(http.errorToString(httpCode));
    }
    http.end();
  } else {
    Serial.println("Not connected to WiFi - cannot call API.");
  }


  mfrc522.PICC_HaltA();
  delay(500);
}
