#include <Wire.h> 
#include <SimpleRotary.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

#define rotaryCCW 5
#define rotaryCW 6
#define rotaryBtn 7

LiquidCrystal_I2C lcd(0x27,20,4);
RTC_DS1307 rtc;
 
// Pin A, Pin B, Button Pin
SimpleRotary rotary(rotaryCW, rotaryCCW, rotaryBtn);
byte rotaryState;
byte rotaryBtnState;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
DateTime now;

void setup() {
 
  lcd.init();
  lcd.backlight();

  rotary.setTrigger(LOW);
  
  
  Serial.begin(9600);  
}

void loop() {
 
  // 0 = not turning, 1 = CW, 2 = CCW
  rotaryState = rotary.rotate();

  if ( rotaryState == 1 ) {
    Serial.println("CW");
  }

  if ( rotaryState == 2 ) {
    Serial.println("CCW");
  }

  rotaryBtnState = rotary.pushType(700);

  if ( rotaryBtnState == 1 ) {
    Serial.println(String("Pressed: ") + "Short");
    displayMainScreen();
  }

  if ( rotaryBtnState == 2 ) {
    Serial.println(String("Pressed: ") + "Long");
  }
//
//  displayMainScreen();
   
//  delay(10);





}

String formatDateNumber(int number) {
  String formattedNumber = String(number);
  return number < 10 ? "0" + formattedNumber : formattedNumber;
}

void displayMainScreen() {
  now = rtc.now();
  lcd.setCursor(6,0);
  lcd.print(
    formatDateNumber(now.hour())
    + String(":")
    + formatDateNumber(now.minute())
    + String(":")
    + formatDateNumber(now.second()));
  lcd.setCursor(0,1);
  lcd.print(
    daysOfTheWeek[now.dayOfTheWeek()]
    + String(", ")
    + now.year()
    + String("/")
    + formatDateNumber(now.month())
    + String("/")
    + formatDateNumber(now.day()));
  lcd.setCursor(3,2);
  lcd.print("WORKING HOURS");
}
