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

String workingMode[] = {"Temperature", "Interval"};
String settingsMenu[] = {"Clock Date/Time", "Working Mode", "Working Hours", "Interval", "Temperature"};
//byte settingsMenuItemsSize = sizeof(settingsMenuItems[1]) / sizeof(settingsMenuItems[1][0]);

short screenLevel = 1;
bool drawScreenOnce = true;
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
  if ( drawScreenOnce ) {
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
    drawScreenOnce = false;
  }
}

void runMenuSettings() {

  drawSettingsMenu();

  if ( rotaryState == 1 ) { // CW
    if (indicatorRow >= indicatorRowMax) {
      if (menuStart == 3) {   
        indicatorRow = indicatorRowMax;
        printIndicator();
      } else {
        drawScreenOnce = true;
        menuStart = 3;
        menuEnd = 4;
        indicatorRow=1;
        indicatorRowMax=2;  
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
        drawScreenOnce = true;
        menuStart = 0;
        menuEnd = 2;
        indicatorRow=3;
        indicatorRowMax=3;
      }
    } else {
      printIndicator(" ");
      indicatorRow--;
      printIndicator();    
    }
  } else if ( rotaryBtnState == 1 ) {
    drawScreenOnce = true;
    if (menuStart == 0) {
      switch (indicatorRow) {
        case 1:
          screenLevel = 0;
          break;
        case 2:
          screenLevel = 2;
          break;
      }
    }
  }
}

void runHomeScreen() {
  lcd.clear();
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

void runWorkingMode() {
  short i=0;
  if (drawScreenOnce) {
    i=0;
    indicatorRow = 2;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SET WORKING MODE");
    lcd.setCursor(0,2);
    lcd.print("Mode: ");
    lcd.setCursor(6,2);
    lcd.print(workingMode[i]);
    drawScreenOnce = false;
  }
  if ( rotaryState == 1 ) {
    Serial.println(i);
    i++;
    if (i > 1) {
      i=0; 
    }
    lcd.setCursor(6,2);
    lcd.print("                   ");
    lcd.setCursor(6,2);
    lcd.print(workingMode[i]);
    Serial.println(i);
  }
}

void setup() {
 
  lcd.init();
  lcd.noBacklight();

  rotary.setTrigger(LOW);
//  rotary.setDebounceDelay(5);
  
  Serial.begin(9600);  
  
  runMenuSettings();
}

void loop() {
     
  rotaryState = rotary.rotate(); // 0 = not turning, 1 = CW, 2 = CCW
  rotaryBtnState = rotary.pushType(700);

  if ( rotaryBtnState == 1 ) {
    Serial.println(String("Pressed: ") + "Short");
  }

  if ( rotaryBtnState == 2 ) {
    Serial.println(String("Pressed: ") + "Long");
  }

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
