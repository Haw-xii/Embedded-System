#include <Arduino.h>
#include <Wire.h>
#include <MyLCD_I2C.h>

MyLCD_I2C lcd(0x27, 16, 2);   // Alamat I2C, kolom, baris

int jam = 11;
int menit = 24;
int detik = 0;

void setup() {
    lcd.begin();              //Initialize
    lcd.backlight();          //Nyalakan backlight
    lcd.print("11/04/2026"); //Print text
}

void loop() {
    lcd.setCursor(0, 1); 
    lcd.print("Time: ");

    // Format time dengan leading zero
    if (jam < 10) lcd.print('0');
      lcd.print(jam);
      lcd.print(':');
    if (menit < 10) lcd.print('0');
      lcd.print(menit);
      lcd.print(':');
    if (detik < 10) lcd.print('0');
      lcd.print(detik);
      delay(1000);

    //update time
    detik++;
    if (detik == 60) {
      detik = 0;
      menit++;
    }
       if (menit == 60) {
        menit = 0;
        jam++;
       }
         if (jam == 24) {
            jam = 0;
         }
      }
    
        