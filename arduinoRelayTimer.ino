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

byte eepromWorkingModeAdr = 0;
byte eepromTemperatureOnAdr1 = 1;
byte eepromTemperatureOnAdr2 = 2;
byte eepromTemperatureOnAdr3 = 3;
byte eepromTemperatureOffAdr1 = 4;
byte eepromTemperatureOffAdr2 = 5;
byte eepromTemperatureOffAdr3 = 6;

//unsigned long previousMillis = 0;
//unsigned long previousMillis2 = 0;
//const long interval = 500; 

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
DateTime now;

String workingMode[] = {"Temperature", "Interval"};
String workingModeUppercase[] = {"TEMPERATURE", "INTERVAL"};
String settingsMenu[] = {"Clock Date/Time", "Working Mode", "Working Hours", "Interval", "Temperature"};
String onOffItemsUppercase[] = {"ON", "OFF"};
//byte settingsMenuItemsSize = sizeof(settingsMenuItems[1]) / sizeof(settingsMenuItems[1][0]);

byte screenLevel = 1;
bool screenExit = false;
unsigned short menuStart = 0;
unsigned short menuEnd = 2;
short indicatorRow = 1;
unsigned short indicatorRowMax = 3;
char indicatorSign = 126;

void printIndicator(short printIndicatorRow, String text = "") {
  lcd.setCursor(0,printIndicatorRow);
  if (text == "") {
    lcd.print(indicatorSign);
  } else {
    lcd.print(text);
  }
}

void runMenuTemperature() {
  unsigned short indicatorRowTemperature = 2;
  byte temperatureSettingsLevel = 0;
  short temperatureValueOn = 0;
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SET TEMPERATURE");
  printIndicator(indicatorRowTemperature);
  short row=2;
  for (short menuItem=0; menuItem<=1; menuItem++) {
    lcd.setCursor(1,row);
    lcd.print(onOffItemsUppercase[menuItem] + String(": "));
    lcd.setCursor(6,row);
    lcd.print("?*C");
    row++;
  }

  lcd.setCursor(6,indicatorRowTemperature);

  while(!screenExit) {    
//    rotaryBtnLongState = rotary.pushLong(1000);
    rotaryBtnState = rotary.push();
    rotaryState = rotary.rotate();

    if (temperatureSettingsLevel == 0) {
      if (rotaryState == 1) { // CW
        printIndicator(indicatorRowTemperature, " ");
        indicatorRowTemperature++;
        if (indicatorRowTemperature > 3) {
          indicatorRowTemperature = 2;
        }      
        printIndicator(indicatorRowTemperature);
      } else if (rotaryState == 2) { // CCW
        printIndicator(indicatorRowTemperature, " ");
        indicatorRowTemperature--;
        if (indicatorRowTemperature < 2) {
          indicatorRowTemperature = 3;
        }      
        printIndicator(indicatorRowTemperature);
      }
    } else if (temperatureSettingsLevel == 1) {

      if (rotaryState == 1) { // CW
        temperatureValueOn++;
        lcd.print(temperatureValueOn + String("*C  "));
      } else if (rotaryState == 2) { // CCW
        temperatureValueOn--;
        lcd.print(temperatureValueOn + String("*C  "));
      }
      
      lcd.setCursor(6,indicatorRowTemperature);      
    }

    if (rotaryBtnState) {
      temperatureSettingsLevel++;
      if (temperatureSettingsLevel > 1) {
        temperatureSettingsLevel = 0; 
        if (indicatorRowTemperature == 2) {
          lcd.setCursor(3,indicatorRowTemperature);
        } else {
          lcd.setCursor(4,indicatorRowTemperature);
        }
        lcd.print(":");
        printIndicator(indicatorRowTemperature);
      } else if (temperatureSettingsLevel == 1) {
        lcd.setCursor(6,indicatorRowTemperature);
        lcd.print(" *C  ");
        if (indicatorRowTemperature == 2) {
          lcd.setCursor(3,indicatorRowTemperature);
        } else {
          lcd.setCursor(4,indicatorRowTemperature);
        }
        lcd.print(indicatorSign);
        printIndicator(indicatorRowTemperature, " ");
      }
    } else if (rotaryBtnLongState) {
      screenExit = true;
      screenLevel = 1;
    }
  }
}

void drawSettingsMenu() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SETTINGS");
  printIndicator(indicatorRow);
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
          printIndicator(indicatorRow);
        } else {
          menuStart = 3;
          menuEnd = 4;
          indicatorRow=1;
          indicatorRowMax=2;  
          drawSettingsMenu();
        }  
      } else {
        printIndicator(indicatorRow, " ");
        indicatorRow++;
        printIndicator(indicatorRow);
      }
      
    } else if ( rotaryState == 2 ) { // CCW
      if (indicatorRow <= 1) {
        if (menuStart == 0) { 
          indicatorRow = 1;
          printIndicator(indicatorRow);
        } else {
          menuStart = 0;
          menuEnd = 2;
          indicatorRow=3;
          indicatorRowMax=3;
          drawSettingsMenu();
        }
      } else {
        printIndicator(indicatorRow, " ");
        indicatorRow--;
        printIndicator(indicatorRow);    
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
      } else if (menuStart == 3) {
        switch (indicatorRow) {
          case 1:
            screenLevel = 0; // interval
            break;
          case 2:
            screenLevel = 3; // temperature
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
      formatDateNumber(now.day())
      + String("/")
      + formatDateNumber(now.month())
      + String("/")
      + now.year());      
    workingModeValue ? lcd.setCursor(3,3) : lcd.setCursor(2,3);
    lcd.print(workingModeUppercase[workingModeValue] + String(" MODE"));

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
    case 3:
      runMenuTemperature();
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
