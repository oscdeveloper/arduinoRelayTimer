#include <EEPROM.h>
#include <Wire.h> 
#include <SimpleRotary.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include "EEPROMAnything.h"

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

byte eepromWorkingMode = 0;
byte eepromTemperatureOn = 1; // 1-2
byte eepromTemperatureOff = 3; // 3-4

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

bool resetSettingsMenu = true;
byte screenNumber = 1;
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

String formatDateNumber(int number) {
  String formattedNumber = String(number);
  return number < 10 ? "0" + formattedNumber : formattedNumber;
}

void runMenuTemperature() {
  unsigned short indicatorRowTemperature = 2;
  byte temperatureSettingsLevel = 0;
  short temperatureValue = 0;
  short temperatureValueOn;
  short temperatureValueOff;
  EEPROM_readAnything(eepromTemperatureOn, temperatureValueOn);
  EEPROM_readAnything(eepromTemperatureOff, temperatureValueOff);
  // TODO add to setup initial values to eeprom, ex. zero
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SET TEMPERATURE");
  printIndicator(indicatorRowTemperature);
  short row=2;
  for (short menuItem=0; menuItem<=1; menuItem++) {
    lcd.setCursor(1,row);
    lcd.print(onOffItemsUppercase[menuItem] + String(": "));
    lcd.setCursor(6,row);
    lcd.print((menuItem == 0 ? temperatureValueOn : temperatureValueOff) + String("*C"));
    row++;
  }

  lcd.setCursor(6,indicatorRowTemperature);

  while(!screenExit) {    
    
    rotaryBtnState = rotary.pushType(700);
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
        if (indicatorRowTemperature == 2) {
          temperatureValueOn++;
          if (temperatureValueOn > 99) {
            temperatureValueOn = 99;
          }
          temperatureValue = temperatureValueOn;
        } else if (indicatorRowTemperature == 3) {
          temperatureValueOff++;
          if (temperatureValueOff > 99) {
            temperatureValueOff = 99;
          }
          temperatureValue = temperatureValueOff;
        }
        lcd.print(temperatureValue + String("*C  "));
      } else if (rotaryState == 2) { // CCW
        if (indicatorRowTemperature == 2) {
          temperatureValueOn--;
          if (temperatureValueOn < -99) {
            temperatureValueOn = -99;
          }
          temperatureValue = temperatureValueOn;
        } else if (indicatorRowTemperature == 3) {
          temperatureValueOff--;
          if (temperatureValueOff < -99) {
            temperatureValueOff = -99;
          }
          temperatureValue = temperatureValueOff;
        }
        lcd.print(temperatureValue + String("*C  "));
      }
      
      lcd.setCursor(6,indicatorRowTemperature);      
    }

    if (rotaryBtnState == 1) {
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
        lcd.print((indicatorRowTemperature == 2 ? temperatureValueOn : temperatureValueOff) + String("*C  "));
        if (indicatorRowTemperature == 2) {
          lcd.setCursor(3,indicatorRowTemperature);
        } else {
          lcd.setCursor(4,indicatorRowTemperature);
        }
        lcd.print(indicatorSign);
        printIndicator(indicatorRowTemperature, " ");
      }
    } else if (rotaryBtnState == 2) {
      EEPROM_writeAnything(eepromTemperatureOn, temperatureValueOn);
      EEPROM_writeAnything(eepromTemperatureOff, temperatureValueOff);
      screenExit = true;
      screenNumber = 1; // settings
    }
  }
}

void drawSettingsMenu() {
  if (resetSettingsMenu) {
     menuStart = 0;
     menuEnd = 2;
     indicatorRow=1;
     indicatorRowMax=3;
     resetSettingsMenu = false;
  }  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(String("SETTINGS ") + (menuStart == 0 ? 1 : 2) + String("/2"));
  printIndicator(indicatorRow);
  short row=1;
  for (short menuItem=menuStart; menuItem<=menuEnd; menuItem++) {
    lcd.setCursor(1,row);
    lcd.print(settingsMenu[menuItem]);
    row++;
  }
}

void runMenuSettings(bool reset = false) {

  drawSettingsMenu();

  while(!screenExit) {

    rotaryBtnState = rotary.pushType(700);
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
            screenNumber = 0; // home
            break;
          case 2:
            screenNumber = 2; // working type
            break;
        }
      } else if (menuStart == 3) {
        switch (indicatorRow) {
          case 1:
            screenNumber = 0; // interval
            break;
          case 2:
            screenNumber = 3; // temperature
            break;
        }
      }
      screenExit = true;
    } else if (rotaryBtnState == 2) {
      screenNumber = 0; // home
      resetSettingsMenu = true;
      screenExit = true;
    }
  }
}

void runHomeScreen() {
  lcd.clear();
  now = rtc.now();
  byte workingModeValue;
  EEPROM_readAnything(eepromWorkingMode, workingModeValue);
  if (workingModeValue != 0 && workingModeValue != 1) workingModeValue = 0;
  
  while(!screenExit) {

    rotaryBtnState = rotary.pushType(700);
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

    if (rotaryBtnState == 2) {
      screenExit = true;
      screenNumber = 1; // settings
    }
  }
}

void runWorkingMode() {
  byte savedValue;
  EEPROM_readAnything(eepromWorkingMode, savedValue);
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
    
    rotaryBtnState = rotary.pushType(700);
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
    
    if (rotaryBtnState == 2) {
      savedValue = i;
      EEPROM_writeAnything(eepromWorkingMode, savedValue);
      screenExit = true;
      screenNumber = 1;
    }
  }
}



void setup() {
 
  lcd.init();
  lcd.noBacklight();

//  rotary.setTrigger(HIGH);
//  rotary.setErrorDelay(0);
//  rotary.setDebounceDelay(5);
  
  Serial.begin(9600);  
  
//  short temperatureValueOn;
//  short temperatureValueOff;
//  EEPROM_readAnything(eepromTemperatureOn, temperatureValueOn);
//  EEPROM_readAnything(eepromTemperatureOff, temperatureValueOff);
//EEPROM_writeAnything(eepromTemperatureOn, temperatureValueOn);
//      EEPROM_writeAnything(eepromTemperatureOff, temperatureValueOff);

//  Serial.println(temperatureValueOn);
//  Serial.println(temperatureValueOff);
  runHomeScreen();

}

void loop() {

  screenExit = false;

  switch(screenNumber) {
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
}



//byte settingsMenuItemsArraySize(byte menuIndex) {
//  return sizeof(settingsMenuItems[menuIndex]) / sizeof(settingsMenuItems[menuIndex][0])
//}
