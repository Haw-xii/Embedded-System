#ifndef MYLCD_I2C_H
#define MYLCD_I2C_H

#include <Arduino.h>
#include <Wire.h>

// ===== KONSTANTA LCD HD44780 =====
// Commands untuk mengontrol LCD
#define LCD_CLEAR           0x01  // Clear display
#define LCD_HOME            0x02  // Return home
#define LCD_ENTRY_MODE      0x04  // Entry mode set
#define LCD_DISPLAY_CTRL    0x08  // Display control
#define LCD_FUNCTION_SET    0x20  // Function set
#define LCD_SET_CGRAM       0x40  // Set CGRAM address
#define LCD_SET_DDRAM       0x80  // Set DDRAM address

// Entry mode flags
#define LCD_ENTRY_LEFT      0x02  // Increment cursor
#define LCD_ENTRY_RIGHT     0x00  // Decrement cursor

// Display control flags
#define LCD_DISPLAY_ON      0x04  // Display ON
#define LCD_CURSOR_ON       0x02  // Cursor ON
#define LCD_BLINK_ON        0x01  // Blink ON

// Function set flags
#define LCD_4BIT_MODE       0x00  // 4-bit mode
#define LCD_8BIT_MODE       0x10  // 8-bit mode
#define LCD_1LINE           0x00  // 1 line display
#define LCD_2LINE           0x08  // 2 line display
#define LCD_5x8DOTS         0x00  // 5x8 dots font
#define LCD_5x10DOTS        0x04  // 5x10 dots font

// ===== PINOUT I2C to LCD =====
// PCF8574 I2C expander ke LCD mapping:
// Bit 0 -> RS (Register Select)
// Bit 1 -> RW (Read/Write) - kita selalu write (LOW)
// Bit 2 -> EN (Enable)
// Bit 3 -> Backlight
// Bit 4-7 -> Data 4-7 (4-bit mode)

#define LCD_RS  0x01  // Register Select bit
#define LCD_RW  0x02  // Read/Write bit (selalu 0 untuk write)
#define LCD_EN  0x04  // Enable bit
#define LCD_BL  0x08  // Backlight bit

class MyLCD_I2C {
public:
    // Constructor: address I2C, kolom, baris
    MyLCD_I2C(uint8_t address, uint8_t cols, uint8_t rows);

    // Inisialisasi LCD
    void begin();

    // Fungsi dasar display
    void clear();                           // Hapus layar
    void home();                            // Cursor ke 0,0
    void setCursor(uint8_t col, uint8_t row); // Set posisi cursor

    // Print text
    void print(const char* text);           // Print string
    void print(int number);                 // Print angka
    void print(char c);                     // Print karakter

    // Kontrol backlight
    void backlight();                       // Nyalakan backlight
    void noBacklight();                     // Matikan backlight

    // Kontrol display dan cursor
    void display();                         // Nyalakan display
    void noDisplay();                       // Matikan display
    void cursor();                          // Tampilkan cursor
    void noCursor();                        // Sembunyikan cursor
    void blink();                           // Cursor berkedip
    void noBlink();                         // Cursor tidak berkedip

    // Custom character (opsional untuk pembelajaran lanjut)
    void createChar(uint8_t location, uint8_t charmap[]);

private:
    uint8_t _address;     // Alamat I2C (biasanya 0x27 atau 0x3F)
    uint8_t _cols;        // Jumlah kolom (biasanya 16 atau 20)
    uint8_t _rows;        // Jumlah baris (biasanya 2 atau 4)
    uint8_t _backlight;   // Status backlight (LCD_BL atau 0)
    uint8_t _displayCtrl; // Status display control

    // Fungsi-fungsi internal untuk komunikasi I2C
    void i2cWrite(uint8_t data);                    // Kirim 1 byte via I2C
    void send(uint8_t value, uint8_t mode);         // Kirim data/command ke LCD
    void write4bits(uint8_t value);                 // Kirim 4 bit ke LCD
    void pulseEnable(uint8_t data);                 // Pulse enable pin
    void sendCommand(uint8_t cmd);                  // Kirim command
    void sendData(uint8_t data);                    // Kirim data
};

#endif
