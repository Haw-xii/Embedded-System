#ifndef SIMPLE_RC522_H
#define SIMPLE_RC522_H

#include <Arduino.h>
#include <SPI.h>

// Register RC522
#define REG_COMMAND       0x01    // Command register
#define REG_COM_IRQ       0x04    // Interrupt request register
#define REG_ERROR         0x06    // Error register
#define REG_STATUS1       0x07    // Status register 1
#define REG_STATUS2       0x08    // Status register 2
#define REG_FIFO_DATA     0x09    // FIFO data register
#define REG_FIFO_LEVEL    0x0A    // FIFO level register
#define REG_CONTROL       0x0C    // Control register
#define REG_BIT_FRAMING   0x0D    // Bit framing register
#define REG_MODE          0x11    // Mode register
#define REG_TX_CONTROL    0x14    // Transmit control register
#define REG_TX_ASK        0x15    // Transmit ASK register
#define REG_CRC_RESULT_H  0x21    // CRC result register (MSB)
#define REG_CRC_RESULT_L  0x22    // CRC result register (LSB)
#define REG_MOD_WIDTH     0x24    // Modulation width register
#define REG_TIMER_MODE    0x2A    // Timer mode register
#define REG_TIMER_PRESCALER 0x2B  // Timer prescaler register
#define REG_TIMER_RELOAD_H  0x2C  // Timer reload value (MSB)
#define REG_TIMER_RELOAD_L  0x2D  // Timer reload value (LSB)

// Command RC522
#define CMD_IDLE          0x00    // Tidak melakukan aksi
#define CMD_TRANSCEIVE    0x0C    // Transmit dan receive data
#define CMD_SOFTRESET     0x0F    // Reset RC522

// PICC Commands (ISO 14443-3 Type A)
#define PICC_CMD_REQA     0x26    // Request command Type A
#define PICC_CMD_SELECT   0x93    // Anti-collision/Select
#define PICC_CMD_HALT     0x50    // Halt command

class SimpleRC522 {
public:
    // Constructor
    SimpleRC522(uint8_t ssPin, uint8_t rstPin);

    // Inisialisasi modul
    void begin();

    // Deteksi apakah ada kartu baru
    bool isCardPresent();

    // Membaca UID kartu (4 byte)
    bool readCardUID(uint8_t *uid);

    // Halt kartu (stop komunikasi)
    void halt();

    // Fungsi untuk debug - print versi firmware
    void printVersion();

private:
    uint8_t _ssPin;
    uint8_t _rstPin;

    // Fungsi SPI dasar
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);

    // Fungsi komunikasi dengan kartu
    uint8_t transceiveCard(uint8_t command, uint8_t *sendData, uint8_t sendLen,
                           uint8_t *backData, uint8_t *backLen);

    // Fungsi anti-collision untuk mendapatkan UID
    uint8_t anticollision(uint8_t *uid);

    // Clear bit dalam register
    void clearBitMask(uint8_t reg, uint8_t mask);

    // Set bit dalam register
    void setBitMask(uint8_t reg, uint8_t mask);
};

#endif
