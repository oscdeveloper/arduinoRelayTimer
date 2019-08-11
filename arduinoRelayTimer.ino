#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

LiquidCrystal_I2C lcd(0x27,20,4);
RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

DateTime now;

unsigned int pinRelay = 11;

bool relay = false;

unsigned long time1;

unsigned int counter = 0;

void setup() {
 
  lcd.init();
  lcd.backlight();
    
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
    + String(" ")
    + now.year()
    + String("/")
    + formatDateNumber(now.month())
    + String("/")
    + formatDateNumber(now.day()));

    lcd.setCursor(3,2);
    lcd.print("WORKING HOURS");

//  pinMode (pinRelay, OUTPUT);
//  
//  digitalWrite(pinRelay, LOW);
//  // LOW state activate relay / switched on
//  delay(5000);
//
//  time1 = millis();
  
//  Serial.begin(9600);  
  
}

void loop() {


//  Serial.print(counter);
//  Serial.print("\t");

//
//if ( counter < 120 ) {
//  
//  digitalWrite(pinRelay, HIGH); // turn off
//  delay(10000);
//  
//  digitalWrite(pinRelay, LOW); // turn on
//  delay(5000);
//  
//  counter++;
//
//} else {
//  digitalWrite(pinRelay, HIGH); // turn off
//  delay(60000);
//}



//  if ( counter == 1 && ( ( millis() - time1 ) > 1500 ) ) {      
//    relay1 = !relay1;
//    digitalWrite(pinRelay1, relay1);
//    counter = 0;      
//  }

//  Serial.print(relay1);
//  Serial.print("\t");
//  Serial.println(relay2);

}

String formatDateNumber(int number) {
  String formattedNumber = String(number);
  return number < 10 ? "0" + formattedNumber : formattedNumber;
}
