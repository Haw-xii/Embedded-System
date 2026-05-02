#include "MyLCD_I2C_Bare.h"

// ===== CONSTRUCTOR =====
MyLCD_I2C_Bare::MyLCD_I2C_Bare(uint8_t address, uint8_t cols, uint8_t rows) {
    _address = address;
    _cols = cols;
    _rows = rows;
    _backlight = LCD_BL;
    _displayCtrl = LCD_DISPLAY_ON | LCD_CURSOR_ON;
}

// ===== TWI/I2C IMPLEMENTATION MANUAL =====

/*
 * Inisialisasi TWI/I2C
 * Untuk Arduino Uno (ATmega328P) @ 16MHz
 * SCL frequency = CPU_CLK / (16 + 2*TWBR*Prescaler)
 * Untuk 100kHz: TWBR = 72, Prescaler = 1
 */
void MyLCD_I2C_Bare::twiInit() {
    // Set SCL frequency ke 100kHz
    // F_SCL = F_CPU / (16 + 2*TWBR*Prescaler)
    // 100000 = 16000000 / (16 + 2*TWBR*1)
    // TWBR = 72
    TWBR = 72;

    // Set prescaler = 1 (TWPS1:TWPS0 = 00)
    TWSR = 0x00;

    // Enable TWI
    TWCR = (1 << TWEN);
}

/*
 * Get TWI Status Code
 * Status ada di bit 7-3 dari TWSR (masking bit 2-0)
 */
uint8_t MyLCD_I2C_Bare::twiGetStatus() {
    return (TWSR & 0xF8);  // Mask prescaler bits
}

/*
 * Send START condition
 * 1. Set TWSTA dan TWEN
 * 2. Wait TWINT flag
 * 3. Check status code
 */
void MyLCD_I2C_Bare::twiStart() {
    // Send START condition
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    // Wait for TWINT flag (START transmitted)
    while (!(TWCR & (1 << TWINT)));

    // Check status (should be 0x08 or 0x10)
    // 0x08 = START transmitted
    // 0x10 = Repeated START transmitted
}

/*
 * Send STOP condition
 */
void MyLCD_I2C_Bare::twiStop() {
    // Send STOP condition
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);

    // Wait for STOP to complete (TWSTO cleared by hardware)
    while (TWCR & (1 << TWSTO));
}

/*
 * Write byte via TWI
 * 1. Load data ke TWDR
 * 2. Set TWINT untuk start transmission
 * 3. Wait TWINT flag
 * 4. Check ACK
 */
void MyLCD_I2C_Bare::twiWrite(uint8_t data) {
    // Load data to data register
    TWDR = data;

    // Clear TWINT to start transmission
    TWCR = (1 << TWINT) | (1 << TWEN);

    // Wait for TWINT flag (data transmitted)
    while (!(TWCR & (1 << TWINT)));

    // Check status (should be 0x18 for SLA+W ACK or 0x28 for DATA ACK)
}

// ===== INISIALISASI LCD =====

void MyLCD_I2C_Bare::begin() {
    // Inisialisasi I2C terlebih dahulu
    twiInit();

    _delay_ms(50);  // Wait >40ms setelah power on

    // Langkah inisialisasi 4-bit mode
    write4bits(0x03 << 4);
    _delay_us(4500);

    write4bits(0x03 << 4);
    _delay_us(4500);

    write4bits(0x03 << 4);
    _delay_us(150);

    // Set ke 4-bit mode
    write4bits(0x02 << 4);
    _delay_us(150);

    // Function set: 4-bit, 2 lines, 5x8
    sendCommand(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8DOTS);

    // Display control
    _displayCtrl = LCD_DISPLAY_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);

    // Clear display
    clear();

    // Entry mode
    sendCommand(LCD_ENTRY_MODE | LCD_ENTRY_LEFT);

    _delay_ms(10);
}

// ===== FUNGSI DASAR DISPLAY =====

void MyLCD_I2C_Bare::clear() {
    sendCommand(LCD_CLEAR);
    _delay_us(2000);
}

void MyLCD_I2C_Bare::home() {
    sendCommand(LCD_HOME);
    _delay_us(2000);
}

void MyLCD_I2C_Bare::setCursor(uint8_t col, uint8_t row) {
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};

    if (row >= _rows) row = _rows - 1;
    if (col >= _cols) col = _cols - 1;

    sendCommand(LCD_SET_DDRAM | (col + row_offsets[row]));
}

// ===== PRINT FUNCTIONS =====

void MyLCD_I2C_Bare::print(const char* text) {
    while (*text) {
        sendData(*text++);
    }
}

void MyLCD_I2C_Bare::print(int number) {
    char buffer[12];
    itoa(number, buffer, 10);
    print(buffer);
}

void MyLCD_I2C_Bare::print(char c) {
    sendData(c);
}

// ===== BACKLIGHT CONTROL =====

void MyLCD_I2C_Bare::backlight() {
    _backlight = LCD_BL;
    i2cWrite(0);
}

void MyLCD_I2C_Bare::noBacklight() {
    _backlight = 0;
    i2cWrite(0);
}

// ===== DISPLAY CONTROL =====

void MyLCD_I2C_Bare::display() {
    _displayCtrl |= LCD_DISPLAY_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);
}

void MyLCD_I2C_Bare::noDisplay() {
    _displayCtrl &= ~LCD_DISPLAY_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);
}

void MyLCD_I2C_Bare::cursor() {
    _displayCtrl |= LCD_CURSOR_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);
}

void MyLCD_I2C_Bare::noCursor() {
    _displayCtrl &= ~LCD_CURSOR_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);
}

void MyLCD_I2C_Bare::blink() {
    _displayCtrl |= LCD_BLINK_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);
}

void MyLCD_I2C_Bare::noBlink() {
    _displayCtrl &= ~LCD_BLINK_ON;
    sendCommand(LCD_DISPLAY_CTRL | _displayCtrl);
}

// ===== CUSTOM CHARACTER =====

void MyLCD_I2C_Bare::createChar(uint8_t location, uint8_t charmap[]) {
    location &= 0x7;
    sendCommand(LCD_SET_CGRAM | (location << 3));
    for (int i = 0; i < 8; i++) {
        sendData(charmap[i]);
    }
    setCursor(0, 0);
}

// ===== I2C COMMUNICATION =====

/*
 * Kirim byte via I2C ke PCF8574
 * Protocol: START - SLA+W - DATA - STOP
 */
void MyLCD_I2C_Bare::i2cWrite(uint8_t data) {
    twiStart();                          // Send START
    twiWrite(_address << 1);             // Send address + Write bit (0)
    twiWrite(data | _backlight);         // Send data + backlight status
    twiStop();                           // Send STOP
}

void MyLCD_I2C_Bare::pulseEnable(uint8_t data) {
    i2cWrite(data | LCD_EN);   // EN = HIGH
    _delay_us(1);              // Pulse >450ns

    i2cWrite(data & ~LCD_EN);  // EN = LOW
    _delay_us(50);             // Command >37us
}

void MyLCD_I2C_Bare::write4bits(uint8_t value) {
    i2cWrite(value);
    pulseEnable(value);
}

void MyLCD_I2C_Bare::send(uint8_t value, uint8_t mode) {
    uint8_t high = value & 0xF0;
    uint8_t low = (value << 4) & 0xF0;

    write4bits(high | mode);
    write4bits(low | mode);
}

void MyLCD_I2C_Bare::sendCommand(uint8_t cmd) {
    send(cmd, 0);
}

void MyLCD_I2C_Bare::sendData(uint8_t data) {
    send(data, LCD_RS);
}
