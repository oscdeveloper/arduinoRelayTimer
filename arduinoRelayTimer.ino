#include <EEPROM.h>
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
byte rotaryBtnLongState;

unsigned short eepromWorkingModeAdr = 0;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
DateTime now;

String workingMode[] = {"Temperature", "Interval"};
String settingsMenu[] = {"Clock Date/Time", "Working Mode", "Working Hours", "Interval", "Temperature"};
//byte settingsMenuItemsSize = sizeof(settingsMenuItems[1]) / sizeof(settingsMenuItems[1][0]);

short screenLevel = 1;
bool screenExit = false;
unsigned short menuStart = 0;
unsigned short menuEnd = 2;
unsigned short indicatorRow = 1;
unsigned short indicatorRowMax = 3;
char indicatorSign = 126;

void printIndicator(String text = "") {
  lcd.setCursor(0,indicatorRow);
  if (text == "") {
    lcd.print(indicatorSign);
  } else {
    lcd.print(text);
  }
}

void drawSettingsMenu() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SETTINGS");
  printIndicator();
  short row=1;
  for (short menuItem=menuStart; menuItem<=menuEnd; menuItem++) {
    lcd.setCursor(1,row);
    lcd.print(settingsMenu[menuItem]);
    row++;
  }
}

void runMenuSettings() {

  drawSettingsMenu();

  while(!screenExit) {

    rotaryBtnState = rotary.push();
    rotaryState = rotary.rotate();

    if ( rotaryState == 1 ) { // CW
      if (indicatorRow >= indicatorRowMax) {
        if (menuStart == 3) {   
          indicatorRow = indicatorRowMax;
          printIndicator();
        } else {
          menuStart = 3;
          menuEnd = 4;
          indicatorRow=1;
          indicatorRowMax=2;  
          drawSettingsMenu();
        }  
      } else {
        printIndicator(" ");
        indicatorRow++;
        printIndicator();
      }
      
    } else if ( rotaryState == 2 ) { // CCW
      if (indicatorRow <= 1) {
        if (menuStart == 0) { 
          indicatorRow = 1;
          printIndicator();
        } else {
          menuStart = 0;
          menuEnd = 2;
          indicatorRow=3;
          indicatorRowMax=3;
          drawSettingsMenu();
        }
      } else {
        printIndicator(" ");
        indicatorRow--;
        printIndicator();    
      }
    } 
    
    if ( rotaryBtnState == 1 ) {
      if (menuStart == 0) {
        switch (indicatorRow) {
          case 1:
            screenLevel = 0; // home
            break;
          case 2:
            screenLevel = 2; // working type
            break;
        }
      }
      screenExit = true;
    }
  }
}

void runHomeScreen() {
  lcd.clear();
  now = rtc.now();
  byte workingModeValue = EEPROM.read(eepromWorkingModeAdr);
  if (workingModeValue != 0 && workingModeValue != 1) workingModeValue = 0;
  
  while(!screenExit) {

    rotaryBtnState = rotary.push();
    rotaryState = rotary.rotate();
    
    lcd.setCursor(6,0);
    lcd.print(
      formatDateNumber(now.hour())
      + String(":")
      + formatDateNumber(now.minute())
      + String(":")
      + formatDateNumber(now.second()));
    lcd.setCursor(0,1);
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    lcd.setCursor(10,1);
    lcd.print(
      now.year()
      + String("/")
      + formatDateNumber(now.month())
      + String("/")
      + formatDateNumber(now.day()));
    lcd.setCursor(4,2);
    lcd.print("WORKING MODE");
    workingModeValue ? lcd.setCursor(6,3) : lcd.setCursor(5,3);
    lcd.print(workingMode[workingModeValue]);

    if (rotaryBtnState) {
      screenExit = true;
      screenLevel = 1;
    }
  }
}

void runWorkingMode() {
  byte savedValue = EEPROM.read(eepromWorkingModeAdr);
  if (savedValue != 0 && savedValue != 1) savedValue = 0;
  short i = savedValue;
  indicatorRow = 2;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SET WORKING MODE");
  lcd.setCursor(0,2);
  lcd.print("Mode: ");
  lcd.setCursor(6,2);
  lcd.print(workingMode[savedValue]);

  while(!screenExit) {
    
    rotaryBtnState = rotary.push();
    rotaryState = rotary.rotate();
    
    if (rotaryState == 1) { // CW
      i++;
      if (i > 1) i=0;
    }

    if (rotaryState == 2) { // CCW
      i--;
      if (i < 0) i=1;
    }

    if (rotaryState == 1 || rotaryState == 2) {
      lcd.setCursor(6,2);
      lcd.print("                   ");
      lcd.setCursor(6,2);
      lcd.print(workingMode[i]);
    }
    
    if (rotaryBtnState) {
      savedValue = i;
      EEPROM.write(eepromWorkingModeAdr, savedValue);
      screenExit = true;
      screenLevel = 1;
    }
  }
}

void setup() {
 
  lcd.init();
  lcd.noBacklight();

  rotary.setTrigger(LOW);
//  rotary.setErrorDelay(0);
//  rotary.setDebounceDelay(5);
  
  Serial.begin(9600);  
  
  runHomeScreen();
}

void loop() {

  screenExit = false;
//  rotaryState = rotary.rotate(); // 0 = not turning, 1 = CW, 2 = CCW
//  rotaryBtnState = rotary.push();

//  if ( rotaryBtnState == 1 ) {
////    Serial.println(String("Pressed: ") + "Short");
//  }
//
//  if ( rotaryBtnState == 2 ) {
////    Serial.println(String("Pressed: ") + "Long");
//  }

  switch(screenLevel) {
    case 0:
    default:   
      runHomeScreen();
      break;
    case 1:
      runMenuSettings();
      break;
    case 2:
      runWorkingMode();
      break;
  }


//  if ( rotaryBtnState == 1 ) {
//    Serial.println(String("Pressed: ") + "Short");
//    displayMainScreen();
//    lcd.backlight();
//  }
//
//  if ( rotaryBtnState == 2 ) {
//    lcd.noBacklight();
//    Serial.println(String("Pressed: ") + "Long");
////    displaySettingsMenu();
//  }
//
//  displayMainScreen();
   
//  delay(10);





}

String formatDateNumber(int number) {
  String formattedNumber = String(number);
  return number < 10 ? "0" + formattedNumber : formattedNumber;
}



//byte settingsMenuItemsArraySize(byte menuIndex) {
//  return sizeof(settingsMenuItems[menuIndex]) / sizeof(settingsMenuItems[menuIndex][0])
//}
