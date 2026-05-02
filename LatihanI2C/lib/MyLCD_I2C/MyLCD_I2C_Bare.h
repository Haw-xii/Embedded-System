#ifndef MYLCD_I2C_BARE_H
#define MYLCD_I2C_BARE_H

// ===== BARE METAL AVR - TANPA ARDUINO.H =====
// Hanya menggunakan AVR libc headers
#include <avr/io.h>          // Register AVR (PORTB, DDRB, dll)
#include <util/delay.h>      // _delay_ms(), _delay_us()
#include <stdint.h>          // uint8_t, uint16_t, dll
#include <stdlib.h>          // itoa()

// ===== KONSTANTA LCD HD44780 =====
#define LCD_CLEAR           0x01
#define LCD_HOME            0x02
#define LCD_ENTRY_MODE      0x04
#define LCD_DISPLAY_CTRL    0x08
#define LCD_FUNCTION_SET    0x20
#define LCD_SET_CGRAM       0x40
#define LCD_SET_DDRAM       0x80

// Entry mode flags
#define LCD_ENTRY_LEFT      0x02
#define LCD_ENTRY_RIGHT     0x00

// Display control flags
#define LCD_DISPLAY_ON      0x04
#define LCD_CURSOR_ON       0x02
#define LCD_BLINK_ON        0x01

// Function set flags
#define LCD_4BIT_MODE       0x00
#define LCD_8BIT_MODE       0x10
#define LCD_1LINE           0x00
#define LCD_2LINE           0x08
#define LCD_5x8DOTS         0x00
#define LCD_5x10DOTS        0x04

// ===== PINOUT I2C to LCD =====
#define LCD_RS  0x01
#define LCD_RW  0x02
#define LCD_EN  0x04
#define LCD_BL  0x08

// ===== TWI/I2C STATUS CODES =====
#define TWI_START           0x08  // START transmitted
#define TWI_REP_START       0x10  // Repeated START transmitted
#define TWI_MT_SLA_ACK      0x18  // SLA+W transmitted, ACK received
#define TWI_MT_DATA_ACK     0x28  // Data transmitted, ACK received

class MyLCD_I2C_Bare {
public:
    // Constructor
    MyLCD_I2C_Bare(uint8_t address, uint8_t cols, uint8_t rows);

    // Inisialisasi (termasuk I2C)
    void begin();

    // Fungsi dasar display
    void clear();
    void home();
    void setCursor(uint8_t col, uint8_t row);

    // Print functions
    void print(const char* text);
    void print(int number);
    void print(char c);

    // Kontrol backlight
    void backlight();
    void noBacklight();

    // Kontrol display
    void display();
    void noDisplay();
    void cursor();
    void noCursor();
    void blink();
    void noBlink();

    // Custom character
    void createChar(uint8_t location, uint8_t charmap[]);

private:
    uint8_t _address;
    uint8_t _cols;
    uint8_t _rows;
    uint8_t _backlight;
    uint8_t _displayCtrl;

    // TWI/I2C functions - implementasi manual!
    void twiInit();                         // Inisialisasi TWI/I2C
    void twiStart();                        // Send START condition
    void twiStop();                         // Send STOP condition
    void twiWrite(uint8_t data);            // Write byte to I2C
    uint8_t twiGetStatus();                 // Get TWI status

    // I2C communication
    void i2cWrite(uint8_t data);
    void pulseEnable(uint8_t data);
    void write4bits(uint8_t value);
    void send(uint8_t value, uint8_t mode);
    void sendCommand(uint8_t cmd);
    void sendData(uint8_t data);
};

#endif
