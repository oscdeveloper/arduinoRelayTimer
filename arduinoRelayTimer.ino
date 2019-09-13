#include <EEPROM.h>
#include <Wire.h> 
#include <SimpleRotary.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include "EEPROMAnything.h"
#include "Adafruit_SHT31.h"

#define rotaryCCW 5
#define rotaryCW 6
#define rotaryBtn 7

#define pinRelay 8

LiquidCrystal_I2C lcd(0x27,20,4);
RTC_DS1307 rtc;
Adafruit_SHT31 temperatureSensor = Adafruit_SHT31();
 
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

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
DateTime now;

byte workingMode;
String workingModeItems[] = {"Temperature", "Interval"};
String workingModeItemsUppercase[] = {"TEMPERATURE", "INTERVAL"};

String settingsMenu[] = {"Working Mode", "Interval", "Temperature", "Working Hours", "Clock Date/Time"};
String onOffItemsUppercase[] = {"ON", "OFF"};
String dateTimeItems[] = {"Date", "Time"};
byte homeScreens[][3] = {
 {0,7,9}, // [0] temperature
 {0,8,9}, // [1] interval
};

bool workingHoursActive = false;
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

float actualTemperature;
short temperatureValueOn;
short temperatureValueOff;

short workingHoursOnHValue;
short workingHoursOnMValue;
short workingHoursOffHValue;
short workingHoursOffMValue;

// {[actualTime ms [millis()]], [savedTime ms]}
unsigned long timeChrono[][2] = {
  {0,0}, // [0] set temperature -> temp now
  {0,0}, // [1] interval listener
  {0,0}, // [2] temperature listener
  {0,0}, // [3] working hours
  {0,0}, // [4] home screen
  {0,0}, // [5] backlight
};

short intervalOnHValue;
short intervalOnMValue;
short intervalOnSValue;
short intervalOffHValue;
short intervalOffMValue;
short intervalOffSValue;
unsigned long intervalTimeLeft[2];
byte intervalMode = 1;

bool backlightSwitch = true;
unsigned int backlightTimeOn = 10000; // 10 sec

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

float formatTemperatureNumber(float number) {
  return round(number * 10.0) / 10.0;
}

void switchRelay(bool switchOn = true) {
  digitalWrite(pinRelay, switchOn);
}

void backLight() {
  
  timeChrono[5][0] = millis();
  if (rotaryState > 0 || rotaryBtnState > 0){    
    timeChrono[5][1] = timeChrono[5][0];
  }
  
  if (timeChrono[5][0] - timeChrono[5][1] >= backlightTimeOn) {
    if (!backlightSwitch) {
      lcd.noBacklight();
    }
    backlightSwitch = true;
  } else {
    if (backlightSwitch) {
      lcd.backlight();
    }
    backlightSwitch = false;
  }
}

bool checkWorkingHours(bool readRtc = true) {
  if (readRtc) {
    now = rtc.now();   
  }
  unsigned short actualTimeNumber = now.hour()*100 + now.minute()%100;
  unsigned short actualTimeNumberOn = workingHoursOnHValue*100 + workingHoursOnMValue%100;  
  unsigned short actualTimeNumberOff = workingHoursOffHValue*100 + workingHoursOffMValue%100;
  if (actualTimeNumberOn == actualTimeNumberOff) {
    workingHoursActive = true;
  } else if (actualTimeNumberOff - actualTimeNumberOn < 0) {
    if (actualTimeNumber - actualTimeNumberOff > 0) {
      workingHoursActive = actualTimeNumber >= actualTimeNumberOn;       
    } else {
      workingHoursActive = actualTimeNumber < actualTimeNumberOff;  
    }
  } else {
    workingHoursActive = actualTimeNumber >= actualTimeNumberOn && actualTimeNumber < actualTimeNumberOff;
  }
  return workingHoursActive;
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
    backLight();

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
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SET INTERVAL");
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
    backLight();

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
      saveToVariableIntervalTimeLeft();
      screenExit = true;
      screenNumber = 1; // settings
    }    
  }
}

void saveToVariableIntervalTimeLeft() {
  timeChrono[1][1] = millis();
  intervalTimeLeft[0] = intervalOffHValue * 3600000 + intervalOffMValue * 60000 + intervalOffSValue * 1000;
  intervalTimeLeft[1] = intervalOnHValue * 3600000 + intervalOnMValue * 60000 + intervalOnSValue * 1000;
}
  
void runSettingsWorkingHours() {
  unsigned short indicatorRowWorkingHours = 2;
  byte workingHoursSettingsLevel = 0;
  short workingHoursValue;
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SET WORKING HOURS");
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
    backLight();

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

  while(!screenExit) {    
    
    rotaryBtnState = rotary.pushType(700);
    rotaryState = rotary.rotate();
    backLight();

    timeChrono[0][0] = millis();
    if (timeChrono[0][0] - timeChrono[0][1] >= 1000UL) {
      timeChrono[0][1] = timeChrono[0][0];
      lcd.setCursor(0,1);
      lcd.print(String("Temp now ") + formatTemperatureNumber(temperatureSensor.readTemperature()));
      lcd.print(degreeSign);
      lcd.print("C");
      lcd.setCursor(6,indicatorRowTemperature);
    }

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

void runSettingsMenu(bool reset = false) {

  switchRelay(false);

  drawSettingsMenu();

  while(!screenExit) {

    rotaryBtnState = rotary.pushType(700);
    rotaryState = rotary.rotate();
    backLight();

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
            screenNumber = 2; // working mode
            break;
          case 2:
            screenNumber = 5; // interval
            break;
          case 3:
            screenNumber = 3; // temperature
            break;
        }
      } else if (menuStart == 3) {
        switch (indicatorRow) {
          case 1:
            screenNumber = 4; // working hours            
            break;
          case 2:
            screenNumber = 6; // clock            
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

  short i = workingMode;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SET WORKING MODE");
  lcd.setCursor(0,2);
  lcd.print("Mode: ");
  lcd.setCursor(6,2);
  lcd.print(workingModeItems[workingMode]);

  while(!screenExit) {
    
    rotaryBtnState = rotary.pushType(700);
    rotaryState = rotary.rotate();
    backLight();
    
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
      lcd.print(workingModeItems[i]);
    }
    
    if (rotaryBtnState == 2) {
      workingMode = i;
      EEPROM_writeAnything(eepromWorkingMode, workingMode);
      screenExit = true;
      screenNumber = 1;
    }
  }
}

void homeScreenSwitch() {
  rotaryState = rotary.rotate();
  rotaryBtnState = rotary.pushType(700);
  backLight();

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
    screenExit = true;
    screenNumber = homeScreens[workingMode][homeScreenNumber];
  }
      
  if (rotaryBtnState == 2) {
    screenExit = true;
    screenNumber = 1; // settings
  } 
}

void intervalTimerListener(bool drawLcdScreen = false) {
  if (workingMode == 1) { // interval mode setting
    if (workingHoursActive) {
      timeChrono[1][0] = millis();
      unsigned short intervalLeftH;
      unsigned short intervalLeftM;
      unsigned short intervalLeftS;
      long intervalTempDiff = 0;
      long intervalCountDiffTime = timeChrono[1][0] - timeChrono[1][1];
  
      if (intervalCountDiffTime >= intervalTimeLeft[intervalMode]) {
        timeChrono[1][1] = timeChrono[1][0];
        intervalCountDiffTime = timeChrono[1][0] - timeChrono[1][1];
        intervalMode = intervalMode ? 0 : 1;
        
        if (drawLcdScreen) {
          lcd.setCursor(6,0);
          if (intervalMode) {            
            lcd.print("on ");
          } else {            
            lcd.print("off");
          }
        }

        if (intervalMode) {
          switchRelay();
        } else {
          switchRelay(false);
        }
      }
  
      intervalTempDiff = (intervalTimeLeft[intervalMode] - intervalCountDiffTime) / 1000;
      intervalLeftH = (intervalTempDiff > 0 ? (intervalTempDiff / 3600) : 0);
      intervalTempDiff = intervalTempDiff - (intervalLeftH * 3600);
      intervalLeftM = intervalTempDiff > 0 ? (intervalTempDiff / 60) : 0;
      intervalTempDiff = intervalTempDiff - (intervalLeftM * 60);
      intervalLeftS = intervalTempDiff > 0 ? intervalTempDiff : 0;
  
      if (drawLcdScreen) {      
        lcd.setCursor(11,0);
        lcd.print(formatDateNumber(intervalLeftH) + String(":") + formatDateNumber(intervalLeftM) + String(":") + formatDateNumber(intervalLeftS));
      }
    } else {
      switchRelay(false);
    }
  }
}

void temperatureReadListener(bool drawLcdScreen = false) {
  if (workingMode == 0) {
    if (workingHoursActive) { // temperature mode setting
      
      timeChrono[2][0] = millis();
      if (timeChrono[2][0] - timeChrono[2][1] >= 1000UL) {
        timeChrono[2][1] = timeChrono[2][0];
        actualTemperature = formatTemperatureNumber(temperatureSensor.readTemperature());
      
        if (drawLcdScreen) {
          lcd.setCursor(10,0);
          lcd.print(actualTemperature);
        }

        if (temperatureValueOn < temperatureValueOff) {
          if (actualTemperature >= temperatureValueOn && actualTemperature < temperatureValueOff) {
            switchRelay();
          } else {
            switchRelay(false);
          }
        } else {
          if (actualTemperature <= temperatureValueOn && actualTemperature > temperatureValueOff) {
            switchRelay();
          } else {
            switchRelay(false);
          }          
        }
      }      
    } else {
      switchRelay(false);
    }
  }
}

void runHomeScreen() {
  lcd.clear();

  workingMode ? lcd.setCursor(3,3) : lcd.setCursor(2,3);
  lcd.print(workingModeItemsUppercase[workingMode] + String(" MODE"));
  
  while(!screenExit) {

    now = rtc.now();

    timeChrono[4][0] = millis();
    if (timeChrono[4][0] - timeChrono[4][1] >= 500UL) {
      timeChrono[4][1] = timeChrono[4][0];
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
    }

    if (!checkWorkingHours(false)) {
      timeChrono[4][0] = millis();
      if (timeChrono[4][0] - timeChrono[4][1] >= 1000UL) {
        timeChrono[4][1] = timeChrono[4][0];
        lcd.setCursor(0,2);
        lcd.print("OUT OF WORKING HOURS");
        switchRelay(false);
      }
    } else {
      temperatureReadListener();
      intervalTimerListener();
    }
    homeScreenSwitch();
  }
}

void runTemperatureScreen() {  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(String("Temp now: ") + formatTemperatureNumber(temperatureSensor.readTemperature()));
  lcd.print(degreeSign);
  lcd.print("C");
  lcd.setCursor(0,2);
  lcd.print("Temp on:");
  lcd.setCursor(10,2);
  lcd.print(temperatureValueOn);
  lcd.print(degreeSign);
  lcd.print("C");
  lcd.setCursor(0,3);
  lcd.print("Temp off:");
  lcd.setCursor(10,3);
  lcd.print(temperatureValueOff);
  lcd.print(degreeSign);
  lcd.print("C");

  while(!screenExit) {
    if (!checkWorkingHours()) {
      timeChrono[2][0] = millis();
      if (timeChrono[2][0] - timeChrono[2][1] >= 1000UL) {
        timeChrono[2][1] = timeChrono[2][0];
        lcd.setCursor(0,0);
        lcd.print("OUT OF WORKING HOURS");
        switchRelay(false);
      }
    } else {
      temperatureReadListener(true);
      intervalTimerListener();
    }
    homeScreenSwitch();
  }
}

void runIntervalScreen() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Relay");
  lcd.setCursor(6,0);
  if (intervalMode) {
    lcd.print("on ");
  } else {
    lcd.print("off");
  }
  lcd.setCursor(0,2);
  lcd.print("Range on:");
  lcd.setCursor(11,2);
  lcd.print(formatDateNumber(intervalOnHValue) + String(":") + formatDateNumber(intervalOnMValue) + String(":") + formatDateNumber(intervalOnSValue));
  lcd.setCursor(0,3);
  lcd.print("Range off:");
  lcd.setCursor(11,3);
  lcd.print(formatDateNumber(intervalOffHValue) + String(":") + formatDateNumber(intervalOffMValue) + String(":") + formatDateNumber(intervalOffSValue));
  
  while(!screenExit) {
    if (!checkWorkingHours()) {
      timeChrono[1][0] = millis();
      if (timeChrono[1][0] - timeChrono[1][1] >= 1000UL) {
        timeChrono[1][1] = timeChrono[1][0];
        lcd.setCursor(0,0);
        lcd.print("OUT OF WORKING HOURS");
        switchRelay(false);
      }
    } else {
      temperatureReadListener();
      intervalTimerListener(true);
    }
    homeScreenSwitch();
  }
}

void runWorkingHoursScreen() {
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
    if (!checkWorkingHours()) {
      timeChrono[3][0] = millis();
      if (timeChrono[3][0] - timeChrono[3][1] >= 1000UL) {
        timeChrono[3][1] = timeChrono[3][0];
        lcd.setCursor(0,0);
        lcd.print("OUT OF WORKING HOURS");
        switchRelay(false);
      }
    } else {
      temperatureReadListener();
      intervalTimerListener();    
    }
    homeScreenSwitch();
  }  
}

void setup() {

  pinMode(pinRelay, OUTPUT);
 
  lcd.init();
  
  temperatureSensor.begin(0x44);

  rotary.setDebounceDelay(0);
 
//  Serial.begin(9600);  

  EEPROM_readAnything(eepromWorkingMode, workingMode);
  
  EEPROM_readAnything(eepromTemperatureOn, temperatureValueOn);
  EEPROM_readAnything(eepromTemperatureOff, temperatureValueOff);
  
  EEPROM_readAnything(eepromWorkingHoursOnH, workingHoursOnHValue);
  EEPROM_readAnything(eepromWorkingHoursOnM, workingHoursOnMValue);
  EEPROM_readAnything(eepromWorkingHoursOffH, workingHoursOffHValue);
  EEPROM_readAnything(eepromWorkingHoursOffM, workingHoursOffMValue);
  
  EEPROM_readAnything(eepromIntervalOnH, intervalOnHValue);
  EEPROM_readAnything(eepromIntervalOnM, intervalOnMValue);
  EEPROM_readAnything(eepromIntervalOnS, intervalOnSValue);
  EEPROM_readAnything(eepromIntervalOffH, intervalOffHValue);
  EEPROM_readAnything(eepromIntervalOffM, intervalOffMValue);
  EEPROM_readAnything(eepromIntervalOffS, intervalOffSValue);
  saveToVariableIntervalTimeLeft();
  
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
      runSettingsMenu();
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
      runTemperatureScreen();
      break;
    case 8:
      runIntervalScreen();
      break;
    case 9:
      runWorkingHoursScreen();
      break;
  }
}
