#include "MyLCD_I2C.h"

// ===== CONSTRUCTOR =====
MyLCD_I2C::MyLCD_I2C(uint8_t address, uint8_t cols, uint8_t rows) {
    _address = address;
    _cols = cols;
    _rows = rows;
    _backlight = LCD_BL;  // Backlight ON by default
    _displayCtrl = LCD_DISPLAY_ON | LCD_CURSOR_ON;  // Display dan cursor ON
}

// ===== INISIALISASI LCD =====
// Mengikuti prosedur inisialisasi HD44780 (datasheet page 45-46)
void MyLCD_I2C::begin() {
    Wire.begin();  // Inisialisasi I2C
    delay(50);     // Wait >40ms setelah power on

    // Langkah inisialisasi 4-bit mode sesuai datasheet
    // Kirim 0x03 tiga kali untuk reset
    write4bits(0x03 << 4);
    delayMicroseconds(4500);

    write4bits(0x03 << 4);
    delayMicroseconds(4500);

    write4bits(0x03 << 4);
    delayMicroseconds(150);

    // Set ke 4-bit mode
    write4bits(0x02 << 4);
    delayMicroseconds(150);

    // Function set: 4-bit, 2 lines, 5x8 dots
    sendCommand(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8DOTS);

    // Display control: display on, cursor off, blink off
    _displayCtrl = LCD_DISPLAY_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);

    // Clear display
    clear();

    // Entry mode: increment cursor, no shift
    sendCommand(LCD_ENTRY_MODE | LCD_ENTRY_LEFT);

    delay(10);
}

// ===== FUNGSI DASAR DISPLAY =====

void MyLCD_I2C::clear() {
    sendCommand(LCD_CLEAR);
    delayMicroseconds(2000);  // Clear butuh waktu lebih lama
}

void MyLCD_I2C::home() {
    sendCommand(LCD_HOME);
    delayMicroseconds(2000);
}

void MyLCD_I2C::setCursor(uint8_t col, uint8_t row) {
    // Row offset untuk LCD 16x2 atau 20x4
    // Row 0: 0x00, Row 1: 0x40, Row 2: 0x14, Row 3: 0x54
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};

    if (row >= _rows) row = _rows - 1;
    if (col >= _cols) col = _cols - 1;

    sendCommand(LCD_SET_DDRAM | (col + row_offsets[row]));
}

// ===== FUNGSI PRINT =====

void MyLCD_I2C::print(const char* text) {
    while (*text) {
        sendData(*text++);
    }
}

void MyLCD_I2C::print(int number) {
    char buffer[12];  // Cukup untuk int (max 11 digit + null)
    itoa(number, buffer, 10);
    print(buffer);
}

void MyLCD_I2C::print(char c) {
    sendData(c);
}

// ===== KONTROL BACKLIGHT =====

void MyLCD_I2C::backlight() {
    _backlight = LCD_BL;
    i2cWrite(0);  // Update backlight status
}

void MyLCD_I2C::noBacklight() {
    _backlight = 0;
    i2cWrite(0);  // Update backlight status
}

// ===== KONTROL DISPLAY DAN CURSOR =====

void MyLCD_I2C::display() {
    _displayCtrl |= LCD_DISPLAY_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);
}

void MyLCD_I2C::noDisplay() {
    _displayCtrl &= ~LCD_DISPLAY_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);
}

void MyLCD_I2C::cursor() {
    _displayCtrl |= LCD_CURSOR_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);
}

void MyLCD_I2C::noCursor() {
    _displayCtrl &= ~LCD_CURSOR_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);
}

void MyLCD_I2C::blink() {
    _displayCtrl |= LCD_BLINK_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);
}

void MyLCD_I2C::noBlink() {
    _displayCtrl &= ~LCD_BLINK_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);
}

// ===== CUSTOM CHARACTER =====

void MyLCD_I2C::createChar(uint8_t location, uint8_t charmap[]) {
    location &= 0x7;  // Hanya ada 8 lokasi (0-7)
    sendCommand(LCD_SET_CGRAM | (location << 3));
    for (int i = 0; i < 8; i++) {
        sendData(charmap[i]);
    }
    setCursor(0, 0);  // Kembali ke DDRAM
}

// ===== FUNGSI INTERNAL I2C =====

// Kirim 1 byte via I2C ke PCF8574
void MyLCD_I2C::i2cWrite(uint8_t data) {
    Wire.beginTransmission(_address);
    Wire.write(data | _backlight);  // Selalu OR dengan backlight status
    Wire.endTransmission();
}

// Pulse Enable Pin
// Enable pin harus HIGH kemudian LOW untuk latch data
void MyLCD_I2C::pulseEnable(uint8_t data) {
    i2cWrite(data | LCD_EN);   // EN = HIGH
    delayMicroseconds(1);      // Pulse harus >450ns

    i2cWrite(data & ~LCD_EN);  // EN = LOW
    delayMicroseconds(50);     // Command butuh >37us
}

// Kirim 4 bit ke LCD
// Dalam 4-bit mode, kita kirim data lewat pin D4-D7 (bit 4-7)
void MyLCD_I2C::write4bits(uint8_t value) {
    i2cWrite(value);
    pulseEnable(value);
}

// Kirim 8 bit ke LCD (dalam 2x 4-bit)
// mode: 0 = command (RS=0), 1 = data (RS=1)
void MyLCD_I2C::send(uint8_t value, uint8_t mode) {
    uint8_t high = value & 0xF0;        // 4 bit tinggi
    uint8_t low = (value << 4) & 0xF0;  // 4 bit rendah (shift ke kiri)

    // Kirim 4 bit tinggi
    write4bits(high | mode);

    // Kirim 4 bit rendah
    write4bits(low | mode);
}

// Kirim command (RS = 0)
void MyLCD_I2C::sendCommand(uint8_t cmd) {
    send(cmd, 0);  // Mode 0 = command (RS bit = 0)
}

// Kirim data (RS = 1)
void MyLCD_I2C::sendData(uint8_t data) {
    send(data, LCD_RS);  // Mode 1 = data (RS bit = 1)
}
