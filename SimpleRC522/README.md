# SimpleRC522 Library

Library sederhana untuk RFID RC522 dengan fokus pada pembelajaran komunikasi SPI.

## Tujuan
Library ini dibuat untuk:
- Memahami komunikasi SPI secara mendalam
- Menyederhanakan library MFRC522 yang kompleks
- Belajar protokol komunikasi dengan modul RC522

## Arsitektur

### 1. Komunikasi SPI
RC522 menggunakan komunikasi SPI dengan format address khusus:

**Format Address untuk WRITE:**
```
Address = (register << 1) & 0x7E
```
- Bit 0 selalu 0 untuk write
- Bit 7 selalu 0 untuk address

**Format Address untuk READ:**
```
Address = ((register << 1) & 0x7E) | 0x80
```
- Bit 0 selalu 0 untuk read
- Bit 7 selalu 1 untuk read

**Contoh:**
```cpp
// Menulis nilai 0x0F ke register COMMAND (0x01)
writeRegister(0x01, 0x0F);
// SPI mengirim: [0x02, 0x0F]
//               address^  ^value

// Membaca dari register VERSION (0x37)
readRegister(0x37);
// SPI mengirim: [0xEE, 0x00]
//               address^  ^dummy
```

### 2. Proses Deteksi Kartu

Langkah-langkah deteksi kartu RFID:

1. **Kirim REQA (Request Type A)**
   - Command: 0x26
   - Menggunakan 7-bit framing (bukan 8-bit)
   - Membangunkan kartu yang berada dalam idle state

2. **Terima ATQA (Answer to Request)**
   - Kartu merespon dengan 2 byte ATQA
   - Berisi informasi tentang tipe kartu

3. **Timeout Handling**
   - Tunggu interrupt RxIRq (data diterima) atau TimerIRq (timeout)
   - Validasi error register

### 3. Proses Pembacaan UID

Menggunakan anti-collision protocol (ISO 14443-3):

1. **Kirim SELECT Command**
   - Command: 0x93 (CASCADE LEVEL 1)
   - Parameter: 0x20 (NVB = Number of Valid Bits)

2. **Terima UID + BCC**
   - 4 byte UID
   - 1 byte BCC (Block Check Character)

3. **Validasi BCC**
   - BCC = UID[0] XOR UID[1] XOR UID[2] XOR UID[3]
   - Memastikan data tidak corrupt

### 4. Register Penting

**Command Register (0x01)**
- CMD_IDLE (0x00): Tidak ada aksi
- CMD_TRANSCEIVE (0x0C): Transmit dan receive
- CMD_SOFTRESET (0x0F): Reset modul

**Control Registers**
- REG_FIFO_DATA (0x09): Buffer data transmit/receive
- REG_FIFO_LEVEL (0x0A): Jumlah byte di FIFO
- REG_BIT_FRAMING (0x0D): Kontrol bit framing
- REG_COM_IRQ (0x04): Interrupt flags
- REG_ERROR (0x06): Error flags

**Status Registers**
- REG_STATUS1 (0x07): Status komunikasi
- REG_STATUS2 (0x08): Status receiver

## Fungsi Utama

### `begin()`
Inisialisasi modul RC522:
- Hardware reset via pin RST
- Soft reset via command
- Konfigurasi timer (25ms timeout)
- Enable antenna dengan mengaktifkan TX1 dan TX2

### `isCardPresent()`
Deteksi keberadaan kartu:
- Mengirim REQA command
- Menunggu response ATQA
- Return `true` jika kartu terdeteksi

### `readCardUID(uint8_t *uid)`
Membaca 4-byte UID kartu:
- Menjalankan anti-collision procedure
- Validasi dengan BCC
- Menyimpan UID ke buffer

### `halt()`
Menghentikan komunikasi dengan kartu:
- Mengirim HALT command (0x50)
- Membuat kartu kembali ke idle state

## Contoh Penggunaan

```cpp
#include <SPI.h>
#include <SimpleRC522.h>

#define RSTPIN 9
#define SSPIN 10

SimpleRC522 rfid(SSPIN, RSTPIN);
uint8_t uid[4];

void setup() {
  Serial.begin(9600);
  SPI.begin();

  rfid.begin();
  rfid.printVersion();
}

void loop() {
  if (rfid.isCardPresent()) {
    if (rfid.readCardUID(uid)) {
      Serial.print("UID: ");
      for (int i = 0; i < 4; i++) {
        Serial.print(uid[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      rfid.halt();
      delay(1000);
    }
  }
}
```

## Perbedaan dengan Library MFRC522 Original

| Aspek | SimpleRC522 | MFRC522 Original |
|-------|-------------|------------------|
| Kompleksitas | Sederhana, fokus basic | Lengkap, banyak fitur |
| Fungsi | Read UID saja | Read/Write, Authentication, dll |
| Kode | ~250 baris | ~2000+ baris |
| Tujuan | Pembelajaran SPI | Production ready |
| PICC Support | Hanya ISO 14443-3A basic | Multiple PICC types |

## Referensi

- [RC522 Datasheet](https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf)
- [ISO/IEC 14443-3](https://www.iso.org/standard/50942.html)
- SPI Protocol: MOSI, MISO, SCK, SS

## Pin Connection

```
RC522    Arduino Uno
--------------------
SDA   -> Pin 10 (SS)
SCK   -> Pin 13 (SCK)
MOSI  -> Pin 11 (MOSI)
MISO  -> Pin 12 (MISO)
IRQ   -> (tidak digunakan)
GND   -> GND
RST   -> Pin 9
3.3V  -> 3.3V
```

**PENTING**: RC522 bekerja pada 3.3V, pastikan level shifter jika menggunakan board 5V!
