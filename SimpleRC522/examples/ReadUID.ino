/*
 * Contoh Sederhana: Membaca UID Kartu RFID
 *
 * Demonstrasi penggunaan library SimpleRC522 untuk
 * membaca UID kartu RFID/NFC
 */

#include <SPI.h>
#include <SimpleRC522.h>

// Pin definition
#define RST_PIN  9
#define SS_PIN   10

// Inisialisasi objek RC522
SimpleRC522 rfid(SS_PIN, RST_PIN);

// Buffer untuk menyimpan UID
uint8_t cardUID[4];

void setup() {
  Serial.begin(9600);

  // Inisialisasi SPI
  SPI.begin();

  // Inisialisasi RC522
  rfid.begin();

  Serial.println(F("=== SimpleRC522 Read UID Example ==="));
  rfid.printVersion();
  Serial.println(F("Scan kartu RFID..."));
  Serial.println();
}

void loop() {
  // Cek apakah ada kartu
  if (rfid.isCardPresent()) {

    // Baca UID kartu
    if (rfid.readCardUID(cardUID)) {

      Serial.print(F("UID Kartu: "));
      for (uint8_t i = 0; i < 4; i++) {
        if (cardUID[i] < 0x10) {
          Serial.print("0");  // Tambahkan leading zero
        }
        Serial.print(cardUID[i], HEX);
        if (i < 3) Serial.print(":");
      }
      Serial.println();

      // Hentikan komunikasi dengan kartu
      rfid.halt();

      // Delay agar tidak terbaca berulang-ulang
      delay(2000);
    }
  }
}
