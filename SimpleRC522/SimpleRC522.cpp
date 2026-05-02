#include "SimpleRC522.h"

// Constructor
SimpleRC522::SimpleRC522(uint8_t ssPin, uint8_t rstPin) {
    _ssPin = ssPin;
    _rstPin = rstPin;
}

// Inisialisasi modul RC522
void SimpleRC522::begin() {
    // Setup pin
    pinMode(_rstPin, OUTPUT);
    pinMode(_ssPin, OUTPUT);

    digitalWrite(_ssPin, HIGH);  // Deselect RC522
    digitalWrite(_rstPin, HIGH); // Enable RC522

    // Reset RC522
    digitalWrite(_rstPin, LOW);
    delayMicroseconds(10);
    digitalWrite(_rstPin, HIGH);
    delay(50);

    // Soft reset via command
    writeRegister(REG_COMMAND, CMD_SOFTRESET);
    delay(50);

    // Konfigurasi timer: auto-mode dengan 25ms timeout
    writeRegister(REG_TIMER_MODE, 0x8D);
    writeRegister(REG_TIMER_PRESCALER, 0x3E);
    writeRegister(REG_TIMER_RELOAD_L, 30);
    writeRegister(REG_TIMER_RELOAD_H, 0);

    // Konfigurasi transmit
    writeRegister(REG_TX_ASK, 0x40);      // 100% ASK modulation
    writeRegister(REG_MODE, 0x3D);        // CRC preset 0x6363

    // Enable antenna
    setBitMask(REG_TX_CONTROL, 0x03);
}

// Tulis ke register RC522
void SimpleRC522::writeRegister(uint8_t reg, uint8_t value) {
    digitalWrite(_ssPin, LOW);

    // Format address: (reg << 1) & 0x7E untuk write
    SPI.transfer((reg << 1) & 0x7E);
    SPI.transfer(value);

    digitalWrite(_ssPin, HIGH);
}

// Baca dari register RC522
uint8_t SimpleRC522::readRegister(uint8_t reg) {
    digitalWrite(_ssPin, LOW);

    // Format address: ((reg << 1) & 0x7E) | 0x80 untuk read
    SPI.transfer(((reg << 1) & 0x7E) | 0x80);
    uint8_t value = SPI.transfer(0x00);

    digitalWrite(_ssPin, HIGH);
    return value;
}

// Set bit mask di register
void SimpleRC522::setBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = readRegister(reg);
    writeRegister(reg, tmp | mask);
}

// Clear bit mask di register
void SimpleRC522::clearBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = readRegister(reg);
    writeRegister(reg, tmp & (~mask));
}

// Print versi firmware RC522
void SimpleRC522::printVersion() {
    uint8_t version = readRegister(0x37);  // Version register
    Serial.print(F("RC522 Version: 0x"));
    Serial.println(version, HEX);
}

// Deteksi apakah ada kartu RFID
bool SimpleRC522::isCardPresent() {
    uint8_t bufferATQA[2];
    uint8_t bufferSize = sizeof(bufferATQA);

    // Clear interrupt flags
    writeRegister(REG_COM_IRQ, 0x7F);

    // Flush FIFO
    setBitMask(REG_FIFO_LEVEL, 0x80);

    // Idle state
    writeRegister(REG_COMMAND, CMD_IDLE);

    // Kirim REQA (Request Type A) ke FIFO
    writeRegister(REG_FIFO_DATA, PICC_CMD_REQA);

    // Set bit framing untuk 7 bit (REQA menggunakan 7 bit)
    writeRegister(REG_BIT_FRAMING, 0x07);

    // Execute Transceive command
    writeRegister(REG_COMMAND, CMD_TRANSCEIVE);
    setBitMask(REG_BIT_FRAMING, 0x80);  // StartSend=1

    // Tunggu selesai (max ~25ms)
    uint16_t timeout = 2000;
    uint8_t n;
    do {
        n = readRegister(REG_COM_IRQ);
        timeout--;
    } while ((timeout != 0) && !(n & 0x30));  // Wait RxIRq or TimerIRq

    clearBitMask(REG_BIT_FRAMING, 0x80);  // Stop sending

    if (timeout == 0) {
        return false;
    }

    // Cek apakah ada error
    uint8_t errorReg = readRegister(REG_ERROR);
    if (errorReg & 0x1B) {  // BufferOvfl, ParityErr, ProtocolErr
        return false;
    }

    // Cek apakah ada data di FIFO
    n = readRegister(REG_FIFO_LEVEL);
    if (n == 0 || n > 2) {
        return false;
    }

    return true;
}

// Baca UID kartu (4 byte)
bool SimpleRC522::readCardUID(uint8_t *uid) {
    // Panggil anti-collision untuk mendapatkan UID
    uint8_t status = anticollision(uid);
    return (status == 0);
}

// Anti-collision procedure untuk mendapatkan UID
uint8_t SimpleRC522::anticollision(uint8_t *uid) {
    // Clear interrupt
    writeRegister(REG_COM_IRQ, 0x7F);

    // Flush FIFO
    setBitMask(REG_FIFO_LEVEL, 0x80);

    // Idle
    writeRegister(REG_COMMAND, CMD_IDLE);

    // Kirim SELECT command (0x93) dan NVB (0x20 = all bits valid)
    writeRegister(REG_FIFO_DATA, PICC_CMD_SELECT);
    writeRegister(REG_FIFO_DATA, 0x20);

    // Reset bit framing
    writeRegister(REG_BIT_FRAMING, 0x00);

    // Execute Transceive
    writeRegister(REG_COMMAND, CMD_TRANSCEIVE);
    setBitMask(REG_BIT_FRAMING, 0x80);  // Start send

    // Tunggu selesai
    uint16_t timeout = 2000;
    uint8_t n;
    do {
        n = readRegister(REG_COM_IRQ);
        timeout--;
    } while ((timeout != 0) && !(n & 0x30));

    clearBitMask(REG_BIT_FRAMING, 0x80);

    if (timeout == 0) {
        return 1;  // Timeout error
    }

    // Cek error
    uint8_t errorReg = readRegister(REG_ERROR);
    if (errorReg & 0x1B) {
        return 2;  // Error
    }

    // Baca UID dari FIFO (4 byte UID + 1 byte BCC)
    n = readRegister(REG_FIFO_LEVEL);
    if (n != 5) {
        return 3;  // Wrong data length
    }

    // Baca 4 byte UID
    for (uint8_t i = 0; i < 4; i++) {
        uid[i] = readRegister(REG_FIFO_DATA);
    }

    // Baca BCC (Block Check Character) - untuk validasi
    uint8_t bcc = readRegister(REG_FIFO_DATA);

    // Verifikasi BCC (XOR dari 4 byte UID)
    uint8_t bccCheck = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
    if (bcc != bccCheck) {
        return 4;  // BCC error
    }

    return 0;  // Success
}

// Halt kartu (stop komunikasi)
void SimpleRC522::halt() {
    // Clear interrupt
    writeRegister(REG_COM_IRQ, 0x7F);

    // Flush FIFO
    setBitMask(REG_FIFO_LEVEL, 0x80);

    // Idle
    writeRegister(REG_COMMAND, CMD_IDLE);

    // Kirim HALT command (0x50, 0x00)
    writeRegister(REG_FIFO_DATA, PICC_CMD_HALT);
    writeRegister(REG_FIFO_DATA, 0x00);

    // Execute Transceive
    writeRegister(REG_BIT_FRAMING, 0x00);
    writeRegister(REG_COMMAND, CMD_TRANSCEIVE);
    setBitMask(REG_BIT_FRAMING, 0x80);

    // Tunggu selesai
    delay(10);

    clearBitMask(REG_BIT_FRAMING, 0x80);
}

