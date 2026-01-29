
/*
  ESP32 + RFID-RC522 + HTTP POST

  Ovaj program radi sljedeće:
  - Povezuje se na WiFi mrežu
  - Inicijalizira RFID čitač (RC522)
  - Kada korisnik prisloni RFID karticu, očitava se njezin ID
  - Taj ID se šalje putem HTTP POST zahtjeva na server koji pokreće Flask aplikaciju

  Autor: GPT
*/

#include <WiFi.h>            // Ugrađena biblioteka za WiFi na ESP32
#include <HTTPClient.h>      // Omogućuje slanje HTTP zahtjeva
#include <SPI.h>             // Komunikacija s RFID modulom koristi SPI protokol
#include <MFRC522.h>         // Knjižnica za rad s RFID-RC522

// ----------- POSTAVKE WI-FI-a ------------
const char* ssid = "IME_TVOJE_WIFI_MREZE";         // Unesi točno ime tvoje WiFi mreže
const char* password = "TVOJA_WIFI_LOZINKA";       // Unesi lozinku tvoje WiFi mreže

// ----------- URL SERVERA -----------------
const char* serverUrl = "http://192.168.1.100:5000/rfid";  // IP + port Flask servera

// ----------- PINOVI ZA RFID MODUL --------
// SS_PIN je SDA (npr. D5), RST_PIN je reset
#define SS_PIN  5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);  // Inicijalizacija RFID čitača

void setup() {
  Serial.begin(115200);  // Serijska komunikacija za ispis poruka

  SPI.begin();           // Pokreće SPI komunikaciju
  rfid.PCD_Init();       // Inicijalizira RFID čitač

  // Spajanje na WiFi
  WiFi.begin(ssid, password);
  Serial.print("Povezivanje na WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Povezano na mrežu!");
}

void loop() {
  // Provjeri postoji li nova RFID kartica
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(500);  // Mala pauza da se ne zaguši serijska komunikacija
    return;
  }

  // Konvertiraj UID kartice u string (heksadecimalno)
  String id_korisnika = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    id_korisnika += String(rfid.uid.uidByte[i], HEX);
  }

  Serial.print("Očitan ID korisnika (RFID): ");
  Serial.println(id_korisnika);

  // Ako je uređaj spojen na WiFi, šalji podatke na server
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);  // Početak HTTP konekcije
    http.addHeader("Content-Type", "application/json");  // POST payload je JSON

    // Priprema JSON poruke: {"id_korisnika": "value"}
    String payload = "{"id_korisnika": "" + id_korisnika + ""}";
    int httpResponseCode = http.POST(payload);  // Šalje POST zahtjev

    if (httpResponseCode > 0) {
      String response = http.getString();  // Odgovor s poslužitelja
      Serial.println("Server odgovorio: " + response);
    } else {
      Serial.print("Greška u POST zahtjevu: ");
      Serial.println(httpResponseCode);
    }

    http.end();  // Zatvori HTTP konekciju
  } else {
    Serial.println("WiFi nije spojen!");
  }

  delay(3000);  // Pauza 3 sekunde između skeniranja
  rfid.PICC_HaltA();  // Završetak komunikacije s karticom
}
