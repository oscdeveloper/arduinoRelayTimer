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
byte eepromWorkingHoursOnH = 5; // 5-6
byte eepromWorkingHoursOnM = 7; // 7-8
byte eepromWorkingHoursOffH = 9; // 9-10
byte eepromWorkingHoursOffM = 11; // 11-12
byte eepromIntervalOnH = 13; // 13-14
byte eepromIntervalOnM = 15; // 15-16
byte eepromIntervalOnS = 17; // 17-18
byte eepromIntervalOffH = 19; // 19-20
byte eepromIntervalOffM = 21; // 21-22
byte eepromIntervalOffS = 23; // 23-24

//unsigned long previousMillis = 0;
//unsigned long previousMillis2 = 0;
//const long interval = 500; 

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
DateTime now;

byte workingModeValue;
String workingMode[] = {"Temperature", "Interval"};
String workingModeUppercase[] = {"TEMPERATURE", "INTERVAL"};
String settingsMenu[] = {"Clock Date/Time", "Working Mode", "Working Hours", "Interval", "Temperature"};
String onOffItemsUppercase[] = {"ON", "OFF"};
String dateTimeItems[] = {"Date", "Time"};
byte homeScreens[2][3] = {
 {0,7,9}, // temperature
 {0,8,9} // interval
};

short homeScreenNumber = 0;
bool resetSettingsMenu = true;
byte screenNumber = 1;
bool screenExit = false;
unsigned short menuStart = 0;
unsigned short menuEnd = 2;
short indicatorRow = 1;
unsigned short indicatorRowMax = 3;
char indicatorSign = 126;
char degreeSign = 223;

short workingHoursOnHValue;
short workingHoursOnMValue;
short workingHoursOffHValue;
short workingHoursOffMValue;

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

void runSettingsClock() {
  now = rtc.now();
  unsigned short indicatorRowClock = 2;
  byte clockSettingsLevel = 0;
  short clockValue;
  short clockHValue = now.hour();
  short clockMValue = now.minute();
  short clockDayValue = now.day();
  short clockMonthValue = now.month();
  short clockYearValue = now.year();
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SET CLOCK");
  printIndicator(indicatorRowClock);
  short row=2;
  for (short menuItem=0; menuItem<=1; menuItem++) {
    lcd.setCursor(1,row);
    lcd.print(dateTimeItems[menuItem] + String(": "));
    lcd.setCursor(7,row);
    if (row == 2) {
      lcd.print(formatDateNumber(clockDayValue) + String("/") + formatDateNumber(clockMonthValue) + String("/") + formatDateNumber(clockYearValue));
    } else if (row == 3) {
      lcd.print(formatDateNumber(clockHValue) + String(":") + formatDateNumber(clockMValue));
    }
    row++;
  }

  lcd.setCursor(6,indicatorRowClock);

  while(!screenExit) {
    
    rotaryBtnState = rotary.pushType(700);
    rotaryState = rotary.rotate();

    if (clockSettingsLevel == 0) { // on/off menu
      if (rotaryState == 1) { // CW
        printIndicator(indicatorRowClock, " ");
        indicatorRowClock++;
        if (indicatorRowClock > 3) {
          indicatorRowClock = 2;
        }      
        printIndicator(indicatorRowClock);
      } else if (rotaryState == 2) { // CCW
        printIndicator(indicatorRowClock, " ");
        indicatorRowClock--;
        if (indicatorRowClock < 2) {
          indicatorRowClock = 3;
        }      
        printIndicator(indicatorRowClock);
      }
    } else if (clockSettingsLevel == 1) { // Day or Hour
      if (rotaryState == 1) { // CW
        if (indicatorRowClock == 2) { // Day
          clockDayValue++;
          if (clockDayValue > 31) {
            clockDayValue = 1;
          }
          clockValue = clockDayValue;
        } else if (indicatorRowClock == 3) { // HH
          clockHValue++;
          if (clockHValue > 23) {
            clockHValue = 0;
          }
          clockValue = clockHValue;
        }
        lcd.setCursor(7,indicatorRowClock);        
        lcd.print(formatDateNumber(clockValue));
      } else if (rotaryState == 2) { // CCW
        if (indicatorRowClock == 2) { // Day
          clockDayValue--;
          if (clockDayValue < 1) {
            clockDayValue = 31;
          }
          clockValue = clockDayValue;
        } else if (indicatorRowClock == 3) { // Hour
          clockHValue--;
          if (clockHValue < 0) {
            clockHValue = 23;
          }
          clockValue = clockHValue;
        }
        lcd.setCursor(7,indicatorRowClock);        
        lcd.print(formatDateNumber(clockValue));
      }     
    } else if (clockSettingsLevel == 2) { // Month or Minute
      if (rotaryState == 1) { // CW
        if (indicatorRowClock == 2) { // Month
          clockMonthValue++;
          if (clockMonthValue > 12) {
            clockMonthValue = 1;
          }
          clockValue = clockMonthValue;
        } else if (indicatorRowClock == 3) { // Minute
          clockMValue++;
          if (clockMValue > 59) {
            clockMValue = 0;
          }
          clockValue = clockMValue;
        }    
        lcd.setCursor(10,indicatorRowClock);        
        lcd.print(formatDateNumber(clockValue)); 
      } else if (rotaryState == 2) { // CCW
        if (indicatorRowClock == 2) { // Month
          clockMonthValue--;
          if (clockMonthValue < 1) {
            clockMonthValue = 12;
          }
          clockValue = clockMonthValue;
        } else if (indicatorRowClock == 3) { // Minute
          clockMValue--;
          if (clockMValue < 0) {
            clockMValue = 59;
          }
          clockValue = clockMValue;
        }    
        lcd.setCursor(10,indicatorRowClock);        
        lcd.print(formatDateNumber(clockValue)); 
      }     
    } else if (clockSettingsLevel == 3) { // Year
      if (rotaryState == 1) { // CW
        clockYearValue++;
        if (clockYearValue > 2099) {
          clockYearValue = 2000;
        }
        clockValue = clockYearValue;
        lcd.setCursor(13,indicatorRowClock);        
        lcd.print(formatDateNumber(clockValue)); 
      } else if (rotaryState == 2) { // CCW
        clockYearValue--;
        if (clockYearValue < 2000) {
          clockYearValue = 2099;
        }
        clockValue = clockYearValue;
        lcd.setCursor(13,indicatorRowClock);        
        lcd.print(formatDateNumber(clockValue)); 
      }     
    }

    if (rotaryBtnState == 1) {
      clockSettingsLevel++;
      if (indicatorRowClock == 2) {
        if (clockSettingsLevel == 1) { // Day
          lcd.setCursor(6,indicatorRowClock);        
          lcd.print(indicatorSign);
          printIndicator(indicatorRowClock, " ");
        } else if (clockSettingsLevel == 2) { // Month
          lcd.setCursor(9,indicatorRowClock);  
          lcd.print(indicatorSign);
          lcd.setCursor(6,indicatorRowClock);
          lcd.print(" ");
        } else if (clockSettingsLevel == 3) { // Year
          lcd.setCursor(12,indicatorRowClock);  
          lcd.print(indicatorSign);
          lcd.setCursor(9,indicatorRowClock);
          lcd.print("/");
        } else if (clockSettingsLevel > 3) { // back to date/time menu
          clockSettingsLevel = 0; 
          lcd.setCursor(12,indicatorRowClock);  
          lcd.print("/");
          printIndicator(indicatorRowClock);
        }             
      } else if (indicatorRowClock == 3) {
        if (clockSettingsLevel == 1) { // HH
          lcd.setCursor(6,indicatorRowClock);        
          lcd.print(indicatorSign);
          printIndicator(indicatorRowClock, " ");
        } else if (clockSettingsLevel == 2) { // MM
          lcd.setCursor(9,indicatorRowClock);  
          lcd.print(indicatorSign);
          lcd.setCursor(6,indicatorRowClock);
          lcd.print(" ");
        } else if (clockSettingsLevel > 2) { // back to date/time menu
          clockSettingsLevel = 0; 
          lcd.setCursor(9,indicatorRowClock);  
          lcd.print(":");
          printIndicator(indicatorRowClock);
        } 
      }
    } else if (rotaryBtnState == 2) {
      RTC_DS1307::adjust(DateTime(clockYearValue, clockMonthValue, clockDayValue, clockHValue, clockMValue, 0));
      screenExit = true;
      screenNumber = 1; // settings
    }    
  }
}

void runSettingsInterval() {
  unsigned short indicatorRowInterval = 2;
  byte intervalSettingsLevel = 0;
  short intervalValue;
  short intervalOnHValue;
  short intervalOnMValue;
  short intervalOnSValue;
  short intervalOffHValue;
  short intervalOffMValue;
  short intervalOffSValue;
  EEPROM_readAnything(eepromIntervalOnH, intervalOnHValue);
  EEPROM_readAnything(eepromIntervalOnM, intervalOnMValue);
  EEPROM_readAnything(eepromIntervalOnS, intervalOnSValue);
  EEPROM_readAnything(eepromIntervalOffH, intervalOffHValue);
  EEPROM_readAnything(eepromIntervalOffM, intervalOffMValue);
  EEPROM_readAnything(eepromIntervalOffS, intervalOffSValue);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SET INTERVAL");
  lcd.setCursor(6,1);
  lcd.print("HH:MM:SS");
  printIndicator(indicatorRowInterval);
  short row=2;
  for (short menuItem=0; menuItem<=1; menuItem++) {
    lcd.setCursor(1,row);
    lcd.print(onOffItemsUppercase[menuItem] + String(": "));
    lcd.setCursor(6,row);
    if (row == 2) {
      lcd.print(formatDateNumber(intervalOnHValue) + String(":") + formatDateNumber(intervalOnMValue) + String(":") + formatDateNumber(intervalOnSValue));
    } else if (row == 3) {
      lcd.print(formatDateNumber(intervalOffHValue) + String(":") + formatDateNumber(intervalOffMValue) + String(":") + formatDateNumber(intervalOffSValue));
    }
    row++;
  }

  lcd.setCursor(6,indicatorRowInterval);

  while(!screenExit) {
    
    rotaryBtnState = rotary.pushType(700);
    rotaryState = rotary.rotate();

    if (intervalSettingsLevel == 0) { // on/off menu
      if (rotaryState == 1) { // CW
        printIndicator(indicatorRowInterval, " ");
        indicatorRowInterval++;
        if (indicatorRowInterval > 3) {
          indicatorRowInterval = 2;
        }      
        printIndicator(indicatorRowInterval);
      } else if (rotaryState == 2) { // CCW
        printIndicator(indicatorRowInterval, " ");
        indicatorRowInterval--;
        if (indicatorRowInterval < 2) {
          indicatorRowInterval = 3;
        }      
        printIndicator(indicatorRowInterval);
      }
    } else if (intervalSettingsLevel == 1) { // HH
      if (rotaryState == 1) { // CW
        if (indicatorRowInterval == 2) {
          intervalOnHValue++;
          if (intervalOnHValue > 23) {
            intervalOnHValue = 0;
          }
          intervalValue = intervalOnHValue;
        } else if (indicatorRowInterval == 3) {
          intervalOffHValue++;
          if (intervalOffHValue > 23) {
            intervalOffHValue = 0;
          }
          intervalValue = intervalOffHValue;
        }
        lcd.setCursor(6,indicatorRowInterval);        
        lcd.print(formatDateNumber(intervalValue));
      } else if (rotaryState == 2) { // CCW
        if (indicatorRowInterval == 2) {
          intervalOnHValue--;
          if (intervalOnHValue < 0) {
            intervalOnHValue = 23;
          }
          intervalValue = intervalOnHValue;
        } else if (indicatorRowInterval == 3) {
          intervalOffHValue--;
          if (intervalOffHValue < 0) {
            intervalOffHValue = 23;
          }
          intervalValue = intervalOffHValue;
        }
        lcd.setCursor(6,indicatorRowInterval);        
        lcd.print(formatDateNumber(intervalValue));
      }     
    } else if (intervalSettingsLevel == 2) { // MM
      if (rotaryState == 1) { // CW
        if (indicatorRowInterval == 2) {
          intervalOnMValue++;
          if (intervalOnMValue > 59) {
            intervalOnMValue = 0;
          }
          intervalValue = intervalOnMValue;
        } else if (indicatorRowInterval == 3) {
          intervalOffMValue++;
          if (intervalOffMValue > 59) {
            intervalOffMValue = 0;
          }
          intervalValue = intervalOffMValue;
        }    
        lcd.setCursor(9,indicatorRowInterval);        
        lcd.print(formatDateNumber(intervalValue)); 
      } else if (rotaryState == 2) { // CCW
        if (indicatorRowInterval == 2) {
          intervalOnMValue--;
          if (intervalOnMValue < 0) {
            intervalOnMValue = 59;
          }
          intervalValue = intervalOnMValue;
        } else if (indicatorRowInterval == 3) {
          intervalOffMValue--;
          if (intervalOffMValue < 0) {
            intervalOffMValue = 59;
          }
          intervalValue = intervalOffMValue;
        }    
        lcd.setCursor(9,indicatorRowInterval);        
        lcd.print(formatDateNumber(intervalValue)); 
      }     
    } else if (intervalSettingsLevel == 3) { // SS
      if (rotaryState == 1) { // CW
        if (indicatorRowInterval == 2) {
          intervalOnSValue++;
          if (intervalOnSValue > 59) {
            intervalOnSValue = 0;
          }
          intervalValue = intervalOnSValue;
        } else if (indicatorRowInterval == 3) {
          intervalOffSValue++;
          if (intervalOffSValue > 59) {
            intervalOffSValue = 0;
          }
          intervalValue = intervalOffSValue;
        }    
        lcd.setCursor(12,indicatorRowInterval);        
        lcd.print(formatDateNumber(intervalValue)); 
      } else if (rotaryState == 2) { // CCW
        if (indicatorRowInterval == 2) {
          intervalOnSValue--;
          if (intervalOnSValue < 0) {
            intervalOnSValue = 59;
          }
          intervalValue = intervalOnSValue;
        } else if (indicatorRowInterval == 3) {
          intervalOffSValue--;
          if (intervalOffSValue < 0) {
            intervalOffSValue = 59;
          }
          intervalValue = intervalOffSValue;
        }    
        lcd.setCursor(12,indicatorRowInterval);        
        lcd.print(formatDateNumber(intervalValue)); 
      }     
    }

    if (rotaryBtnState == 1) {
      intervalSettingsLevel++;
      if (intervalSettingsLevel == 1) { // HH
        lcd.setCursor(5,indicatorRowInterval);        
        lcd.print(indicatorSign);
        printIndicator(indicatorRowInterval, " ");
      } else if (intervalSettingsLevel == 2) { // MM
        lcd.setCursor(8,indicatorRowInterval);  
        lcd.print(indicatorSign);
        lcd.setCursor(5,indicatorRowInterval);
        lcd.print(" ");
      } else if (intervalSettingsLevel == 3) { // SS
        lcd.setCursor(11,indicatorRowInterval);  
        lcd.print(indicatorSign);
        lcd.setCursor(8,indicatorRowInterval);
        lcd.print(":");
      } else if (intervalSettingsLevel > 3) { // back to on/off menu
        intervalSettingsLevel = 0; 
        lcd.setCursor(11,indicatorRowInterval);  
        lcd.print(":");
        printIndicator(indicatorRowInterval);
      }
    } else if (rotaryBtnState == 2) {
      EEPROM_writeAnything(eepromIntervalOnH, intervalOnHValue);
      EEPROM_writeAnything(eepromIntervalOnM, intervalOnMValue);
      EEPROM_writeAnything(eepromIntervalOnS, intervalOnSValue);
      EEPROM_writeAnything(eepromIntervalOffH, intervalOffHValue);
      EEPROM_writeAnything(eepromIntervalOffM, intervalOffMValue);
      EEPROM_writeAnything(eepromIntervalOffS, intervalOffSValue);
      screenExit = true;
      screenNumber = 1; // settings
    }    
  }
}

void runSettingsWorkingHours() {
  unsigned short indicatorRowWorkingHours = 2;
  byte workingHoursSettingsLevel = 0;
  short workingHoursValue;
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SET WORKING HOURS");
  lcd.setCursor(6,1);
  lcd.print("HH:MM");
  printIndicator(indicatorRowWorkingHours);
  short row=2;
  for (short menuItem=0; menuItem<=1; menuItem++) {
    lcd.setCursor(1,row);
    lcd.print(onOffItemsUppercase[menuItem] + String(": "));
    lcd.setCursor(6,row);
    if (row == 2) {
      lcd.print(formatDateNumber(workingHoursOnHValue) + String(":") + formatDateNumber(workingHoursOnMValue));
    } else if (row == 3) {
      lcd.print(formatDateNumber(workingHoursOffHValue) + String(":") + formatDateNumber(workingHoursOffMValue));
    }
    row++;
  }

  lcd.setCursor(6,indicatorRowWorkingHours);

  while(!screenExit) {
    
    rotaryBtnState = rotary.pushType(700);
    rotaryState = rotary.rotate();

    if (workingHoursSettingsLevel == 0) { // on/off menu
      if (rotaryState == 1) { // CW
        printIndicator(indicatorRowWorkingHours, " ");
        indicatorRowWorkingHours++;
        if (indicatorRowWorkingHours > 3) {
          indicatorRowWorkingHours = 2;
        }      
        printIndicator(indicatorRowWorkingHours);
      } else if (rotaryState == 2) { // CCW
        printIndicator(indicatorRowWorkingHours, " ");
        indicatorRowWorkingHours--;
        if (indicatorRowWorkingHours < 2) {
          indicatorRowWorkingHours = 3;
        }      
        printIndicator(indicatorRowWorkingHours);
      }
    } else if (workingHoursSettingsLevel == 1) { // HH
      if (rotaryState == 1) { // CW
        if (indicatorRowWorkingHours == 2) {
          workingHoursOnHValue++;
          if (workingHoursOnHValue > 23) {
            workingHoursOnHValue = 0;
          }
          workingHoursValue = workingHoursOnHValue;
        } else if (indicatorRowWorkingHours == 3) {
          workingHoursOffHValue++;
          if (workingHoursOffHValue > 23) {
            workingHoursOffHValue = 0;
          }
          workingHoursValue = workingHoursOffHValue;
        }
        lcd.setCursor(6,indicatorRowWorkingHours);        
        lcd.print(formatDateNumber(workingHoursValue));
      } else if (rotaryState == 2) { // CCW
        if (indicatorRowWorkingHours == 2) {
          workingHoursOnHValue--;
          if (workingHoursOnHValue < 0) {
            workingHoursOnHValue = 23;
          }
          workingHoursValue = workingHoursOnHValue;
        } else if (indicatorRowWorkingHours == 3) {
          workingHoursOffHValue--;
          if (workingHoursOffHValue < 0) {
            workingHoursOffHValue = 23;
          }
          workingHoursValue = workingHoursOffHValue;
        }
        lcd.setCursor(6,indicatorRowWorkingHours);        
        lcd.print(formatDateNumber(workingHoursValue));
      }     
    } else if (workingHoursSettingsLevel == 2) { // MM
      if (rotaryState == 1) { // CW
        if (indicatorRowWorkingHours == 2) {
          workingHoursOnMValue++;
          if (workingHoursOnMValue > 59) {
            workingHoursOnMValue = 0;
          }
          workingHoursValue = workingHoursOnMValue;
        } else if (indicatorRowWorkingHours == 3) {
          workingHoursOffMValue++;
          if (workingHoursOffMValue > 59) {
            workingHoursOffMValue = 0;
          }
          workingHoursValue = workingHoursOffMValue;
        }    
        lcd.setCursor(9,indicatorRowWorkingHours);        
        lcd.print(formatDateNumber(workingHoursValue)); 
      } else if (rotaryState == 2) { // CCW
        if (indicatorRowWorkingHours == 2) {
          workingHoursOnMValue--;
          if (workingHoursOnMValue < 0) {
            workingHoursOnMValue = 59;
          }
          workingHoursValue = workingHoursOnMValue;
        } else if (indicatorRowWorkingHours == 3) {
          workingHoursOffMValue--;
          if (workingHoursOffMValue < 0) {
            workingHoursOffMValue = 59;
          }
          workingHoursValue = workingHoursOffMValue;
        }    
        lcd.setCursor(9,indicatorRowWorkingHours);        
        lcd.print(formatDateNumber(workingHoursValue)); 
      }     
    }

    if (rotaryBtnState == 1) {
      workingHoursSettingsLevel++;
      if (workingHoursSettingsLevel == 1) { // HH
        lcd.setCursor(5,indicatorRowWorkingHours);        
        lcd.print(indicatorSign);
        printIndicator(indicatorRowWorkingHours, " ");
      } else if (workingHoursSettingsLevel == 2) { // MM
        lcd.setCursor(8,indicatorRowWorkingHours);  
        lcd.print(indicatorSign);
        lcd.setCursor(5,indicatorRowWorkingHours);
        lcd.print(" ");
      } else if (workingHoursSettingsLevel > 2) { // back to on/off menu
        workingHoursSettingsLevel = 0; 
        lcd.setCursor(8,indicatorRowWorkingHours);  
        lcd.print(":");
        printIndicator(indicatorRowWorkingHours);
      }
    } else if (rotaryBtnState == 2) {
      EEPROM_writeAnything(eepromWorkingHoursOnH, workingHoursOnHValue);
      EEPROM_writeAnything(eepromWorkingHoursOnM, workingHoursOnMValue);
      EEPROM_writeAnything(eepromWorkingHoursOffH, workingHoursOffHValue);
      EEPROM_writeAnything(eepromWorkingHoursOffM, workingHoursOffMValue);
      screenExit = true;
      screenNumber = 1; // settings
    }    
  }
}

void runSettingsTemperature() {
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
    lcd.print(menuItem == 0 ? temperatureValueOn : temperatureValueOff);
    lcd.print(degreeSign);
    lcd.print("C");
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
        lcd.print(temperatureValue);
        lcd.print(degreeSign);
        lcd.print("C  ");
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
        lcd.print(temperatureValue);
        lcd.print(degreeSign);
        lcd.print("C  ");
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
        lcd.print(indicatorRowTemperature == 2 ? temperatureValueOn : temperatureValueOff);
        lcd.print(degreeSign);
        lcd.print("C  ");
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
            screenNumber = 6; // clock
            break;
          case 2:
            screenNumber = 2; // working mode
            break;
          case 3:
            screenNumber = 4; // working hours
            break;
        }
      } else if (menuStart == 3) {
        switch (indicatorRow) {
          case 1:
            screenNumber = 5; // interval
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

void runSettingsWorkingMode() {

  short i = workingModeValue;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SET WORKING MODE");
  lcd.setCursor(0,2);
  lcd.print("Mode: ");
  lcd.setCursor(6,2);
  lcd.print(workingMode[workingModeValue]);

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
      workingModeValue = i;
      EEPROM_writeAnything(eepromWorkingMode, workingModeValue);
      screenExit = true;
      screenNumber = 1;
    }
  }
}

void runHomeScreen() {
  lcd.clear();

  workingModeValue ? lcd.setCursor(3,3) : lcd.setCursor(2,3);
  lcd.print(workingModeUppercase[workingModeValue] + String(" MODE"));
  
  while(!screenExit) {

    now = rtc.now();
        
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

    homeScreenSwitch();
  }
}

void runTemperature() {  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("runTemperature");

  while(!screenExit) {
    homeScreenSwitch();
  }
}

void runInterval() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("runInterval");

  while(!screenExit) {
    homeScreenSwitch();
  }
}

void runWorkingHours() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("WORKING HOURS");
  lcd.setCursor(0,2);
  lcd.print("Start:");
  lcd.setCursor(7,2);
  lcd.print(formatDateNumber(workingHoursOnHValue) + String(":") + formatDateNumber(workingHoursOnMValue));
  lcd.setCursor(0,3);
  lcd.print("End:");
  lcd.setCursor(7,3);
  lcd.print(formatDateNumber(workingHoursOffHValue) + String(":") + formatDateNumber(workingHoursOffMValue));
  
  while(!screenExit) {
    homeScreenSwitch();
  }  
}

void homeScreenSwitch() {
  
  rotaryState = rotary.rotate();
  rotaryBtnState = rotary.pushType(700);

  if (rotaryState == 1) { // CW
    homeScreenNumber++;
    if (homeScreenNumber > 2) {
      homeScreenNumber = 0;
    }
  } else if (rotaryState == 2) { // CCW
    homeScreenNumber--;
    if (homeScreenNumber < 0) {
      homeScreenNumber = 2;
    }
  }

  if (rotaryState > 0) {
    screenNumber = homeScreens[workingModeValue][homeScreenNumber];
    screenExit = true;
  }
      
  if (rotaryBtnState == 2) {
    screenExit = true;
    screenNumber = 1; // settings
  } 
}


void setup() {
 
  lcd.init();
  lcd.noBacklight();
 
  Serial.begin(9600);  

  EEPROM_readAnything(eepromWorkingMode, workingModeValue);
  EEPROM_readAnything(eepromWorkingHoursOnH, workingHoursOnHValue);
  EEPROM_readAnything(eepromWorkingHoursOnM, workingHoursOnMValue);
  EEPROM_readAnything(eepromWorkingHoursOffH, workingHoursOffHValue);
  EEPROM_readAnything(eepromWorkingHoursOffM, workingHoursOffMValue);
  
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
      runSettingsWorkingMode();
      break;
    case 3:
      runSettingsTemperature();
      break;
    case 4:
      runSettingsWorkingHours();
      break;
    case 5:
      runSettingsInterval();
      break;
    case 6:
      runSettingsClock();
      break;
    case 7:
      runTemperature();
      break;
    case 8:
      runInterval();
      break;
    case 9:
      runWorkingHours();
      break;
  }
// byte homeScreensNumber[] = {0,7,8,9};
}



//byte settingsMenuItemsArraySize(byte menuIndex) {
//  return sizeof(settingsMenuItems[menuIndex]) / sizeof(settingsMenuItems[menuIndex][0])
//}
