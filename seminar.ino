
#include <WiFi.h>           
#include <HTTPClient.h>    
#include <SPI.h>           
#include <MFRC522.h>

const char* ssid = "";    
const char* password = "";    

const char* serverUrl = "http://192.168.1.100:5000/rfid"; 

#define SS_PIN  5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200); 

  SPI.begin();      
  rfid.PCD_Init(); 

  WiFi.begin(ssid, password);
  Serial.print("Povezivanje na WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Povezano na mrežu!");
}

void loop() {

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(500);
    return;
  }

  String id_korisnika = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    id_korisnika += String(rfid.uid.uidByte[i], HEX);
  }

  Serial.print("Očitan ID korisnika (RFID): ");
  Serial.println(id_korisnika);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl); 
    http.addHeader("Content-Type", "application/json");


    String payload = "{"id_korisnika": "" + id_korisnika + ""}";
    int httpResponseCode = http.POST(payload); 

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server odgovorio: " + response);
    } else {
      Serial.print("Greška u POST zahtjevu: ");
      Serial.println(httpResponseCode);
    }

    http.end(); 
  } else {
    Serial.println("WiFi nije spojen!");
  }

  delay(3000); 
  rfid.PICC_HaltA(); 
}
