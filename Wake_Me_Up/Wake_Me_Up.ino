#include <Wire.h> // Include wire library, which facilitates serial communication with lcd.
#include <LiquidCrystal_I2C.h> // Include I2C library for lcd screen.
#include <EEPROM.h> // Include library that allows us to write data to memory.

LiquidCrystal_I2C lcd(0x3F, 20, 4); // Initialize new object called lcd.

#define aoJoyXVal A2 // X val on joy.
#define aoJoyYVal A3 // Y val on joy.
#define diJoyPress 3 // Btn on joystick.
#define diBtnToggleAlarmAdri 6 // Btn to toggle alarm.

int gloTimeMillis = millis(); // Will keep track of how many milliseconds have passed since start of program.
int gloPrevTimeMillis = gloTimeMillis; // Val will keep track of difference between last noted time and current time.
int gloJoyHeldMillis = 0; // Will keep track of how long joystick is being held.
int gloJoyPressMillis = 0; // Will keep track of how long joy button is held.
int gloPrevAlarmDigitTimeMillis = 0; // Timer keeps track of when to blink alarm digit.

int gloCountSeconds = 0; // Seconds(not used but prevent bug where gloPrevTimeMillis goes negative around 27000).
int gloCountMinutes = 0; // Minutes.
int gloCountHours = 0; // Hours.
int gloClockMode = 0; // What mode clock is in(0 is display time, 1 is change alarm).

int gloAlarmMinutes = 0; // First digit of minute alarm is supposed to go off.
int gloAlarmMinutes2 = 0; // Second digit.
int gloAlarmHours = 0; // First digit of hour alarm is supposed to go off.
int gloAlarmHours2 = 0; // Second digit.
int gloAlarmDigitSelected = 0; // Keeps track of selected digit(one user is changing), from right to left, so it knows which one not to blink.
int gloDoAlarm = 1; // Whether alarm should fire or not.

int gloJoyPressVal = 0; // Value to keep track if joystick button has been pressed.
int gloJoyPressVal2 = 0; // Value to debounce.
int gloJoyButtonState = 0; // Keeps track of state button is in.
int gloJoyStickValX = 0; // Value to keep track of stick movement.
int gloJoyStickValY = 0; // Value to debounce.
int gloJoyStickState = 0; // Keeps track of state stick is in.

int gloBtnToggleAlarmVal = 0; // Value to keep track of toggle alarm button press.
int gloBtnToggleAlarmVal2 = 0; // Debounce value.
int gloBtnToggleAlarmState = 0; // Keeps track of state button is in.

// Initial code at startup.
void setup() {
  Serial.begin(9600); // Open serial console.
  pinMode(diJoyPress, INPUT); // Define joystick button.
  pinMode(diBtnToggleAlarmAdri, INPUT); // Define toggle alarm button.
  digitalWrite(diJoyPress, HIGH); // Write joystick button high(default, unpressed value is high).
  lcd.init(); // initialize the lcd.
  lcd.backlight(); // Turns on backlight, otherwise lcd screen is very dark.
  gloAlarmMinutes = EEPROM.read(0); // Get alarm minute from memory.
  gloAlarmMinutes2 = EEPROM.read(1); // Get alarm minute2.
  gloAlarmHours = EEPROM.read(2); // Get hour.
  gloAlarmHours2 = EEPROM.read(3); // Get hour2.
}

// Main loop.
void loop() {
  // If clock mode is 0, perform logic to display current time.
  if (gloClockMode == 0) {
    MethodShowTime();
  }
  // Else if clock mode is 1, perform logic to change alarm time and show alarm on screen.
  else if (gloClockMode == 1) {
    MethodChangeAlarm();
    MethodShowAlarm();
  }

  MethodBtnToggleAlarmPress(); // Runs timer to toggle alarm on or off.
  MethodTmrTick(); // Runs method that makes timer tick.
  MethodJoyPress(); // Runs method that checks whether joystick button has been pressed.
}

// Method which will check if current time matches alarm time and will run alarm sound if it matches.
void MethodAlarmCheck() {
  int locAlarmMinutes = (String(gloAlarmMinutes2) + String(gloAlarmMinutes)).toInt(); // Combine two seperate alarm minute values.
  int locAlarmHours = (String(gloAlarmHours2) + String(gloAlarmHours)).toInt(); // Combine hour values.

  // If both match, sound alarm.
  if (locAlarmMinutes == gloCountMinutes && locAlarmHours == gloCountHours) {
    Serial.println("Alarm is going to sound."); // 
    MethodDoAlarm(); // Run method to make alarm go off.
  }
}

void MethodBtnToggleAlarmPress() {
  gloBtnToggleAlarmVal = digitalRead(diBtnToggleAlarmAdri); // Read btn value.
  delay(10); // Delay
  gloBtnToggleAlarmVal2 = digitalRead(diBtnToggleAlarmAdri); // Read it again.

  //Check whether results were same(debounce function) and previous state. run timer method, which will start a 2 second timer, if it is held for 2 seconds change clockMode.
  if (gloBtnToggleAlarmVal == gloBtnToggleAlarmVal2) {
    // toggle alarm status if on first button press.
    if (gloDoAlarm == 1 && gloBtnToggleAlarmState != gloBtnToggleAlarmVal && gloBtnToggleAlarmVal == LOW) {
      Serial.println("Toggle alarm button pressed, turning alarm off.");
      gloDoAlarm = 0; // Turn off alarm.
    }
    else if (gloDoAlarm == 0 && gloBtnToggleAlarmState != gloBtnToggleAlarmVal && gloBtnToggleAlarmVal == LOW) {
      Serial.println("Toggle alarm button pressed, turning alarm on.");
      gloDoAlarm = 1; // Turn on alarm.
    }
    
    gloBtnToggleAlarmState = gloBtnToggleAlarmVal; // Update button state.
  }
}

// Empty method that will sound alarm.
void MethodDoAlarm() {
  
}

// Method which displays the alarm time.
void MethodShowAlarm() {
  String locAlarmString = ""; // String which will be send to write to lcd function.

  gloTimeMillis = millis(); // Get time.

  // Every 400ms, blink digit for 400ms, to indicate which character user is changing. This will create a blinking effect to indicate to user he is changing alarm.
  if (gloTimeMillis - gloPrevAlarmDigitTimeMillis > 400) {
    if (gloAlarmDigitSelected == 0) {
      locAlarmString = "  : " + String(gloAlarmMinutes); // Only display furthest right digit.
    }
    else if (gloAlarmDigitSelected == 1) {
      locAlarmString = "  :" + String(gloAlarmMinutes2) + " ";
    }
    else if (gloAlarmDigitSelected == 2) {
      locAlarmString = " " + String(gloAlarmHours) + ":  ";
    }
    else if (gloAlarmDigitSelected == 3) {
      locAlarmString = String(gloAlarmHours2) + " :  ";
    }

    // If 800 millis have passed, reset time back to 0.
    if (gloTimeMillis - gloPrevAlarmDigitTimeMillis > 800) {
      gloPrevAlarmDigitTimeMillis = gloTimeMillis; // Update time
    }
  }
  // Else display all character.
  else {
    locAlarmString = String(gloAlarmHours2) + String(gloAlarmHours) + ":" + String(gloAlarmMinutes2) + String(gloAlarmMinutes);
  }
  
  MethodWriteToLcd(0, 0, locAlarmString); // Display the created string.
}

// Method which will read joystick analog values, and perform logic to change alarm time as a consequence.
void MethodChangeAlarm() {
  gloJoyStickValX = analogRead(aoJoyXVal); // X val.
  gloJoyStickValY = analogRead(aoJoyYVal); // Y val.
  gloTimeMillis = millis(); // Get time.

  // Check what position the joystick is in and if it has already performed an action in previous cycle(gloJoyStickState). Joystick is right direction.
  if (gloJoyStickValX > 750 && gloJoyStickState == 0) {
    // If there is a digit further to the right, select digit on the right.
    if (gloAlarmDigitSelected > 0) {
      gloAlarmDigitSelected--;
    }

    gloJoyHeldMillis = gloTimeMillis;
    gloJoyStickState = 1;
  }
  // Joystick is left direction, select digit to the left.
  else if (gloJoyStickValX < 250 && gloJoyStickState == 0) {
    // If there is a digit to the left, select digit to the left.
    if (gloAlarmDigitSelected < 3) {
      gloAlarmDigitSelected++;
    }

    gloJoyHeldMillis = gloTimeMillis;
    gloJoyStickState = 1;
  }
  // Joystick is down direction, subtract 1 of current digit and perform check whether it will go below 0 if a digit were to be subtracted, and if it does, loop it around to max value, otherwise subtract 1.
  else if (gloJoyStickValY > 750 && gloJoyStickState == 0) {
    if (gloAlarmDigitSelected == 0) {
      if (gloAlarmMinutes <= 0) {
        gloAlarmMinutes = 9; // Max is 9 for digit 0.
      }
      else {
        gloAlarmMinutes--;
      }
    }
    else if (gloAlarmDigitSelected == 1) {
      if (gloAlarmMinutes2 <= 0) {
        gloAlarmMinutes2 = 5; // Max is 5 for digit 1.
      }
      else {
        gloAlarmMinutes2--;
      }
    }
    else if (gloAlarmDigitSelected == 2) {
      if (gloAlarmHours <= 0 && gloAlarmHours2 >= 2) {
        gloAlarmHours = 3; // Max is 3 if digit 3 is 2
      }
      else if (gloAlarmHours <= 0 && gloAlarmHours2 <= 1){
        gloAlarmHours = 9; // Max is 9 if digit 3 is < 2
      }
      else {
        gloAlarmHours--;
      }
    }
    else if (gloAlarmDigitSelected == 3) {
      if (gloAlarmHours2 <= 0) {
        gloAlarmHours2 = 2; // Max is 2 for digit 3.

        // If digit 2 is above or equal to 4, set it to 3, which is the max value if digit 3 is 2.
        if (gloAlarmHours >= 4) {
          gloAlarmHours = 3;
        }
      }
      else {
        gloAlarmHours2--;
      }
    }
    
    gloJoyHeldMillis = gloTimeMillis;
    gloJoyStickState = 1;
  }
  // Joystick is up direction, add 1 to current digit and perform check whether it will go below max value if 1 were to be added, and if it does, loop it around to min value(0), otherwise add 1, max values are found above.
  else if (gloJoyStickValY < 250 && gloJoyStickState == 0) {
    if (gloAlarmDigitSelected == 0) {
      if (gloAlarmMinutes >= 9) {
        gloAlarmMinutes = 0;
      }
      else {
        gloAlarmMinutes++;
      }
    }
    else if (gloAlarmDigitSelected == 1) {
      if (gloAlarmMinutes2 >= 5) {
        gloAlarmMinutes2 = 0;
      }
      else {
        gloAlarmMinutes2++;
      }
    }
    else if (gloAlarmDigitSelected == 2) {
      if (gloAlarmHours >= 9 && gloAlarmHours2 < 2) {
        gloAlarmHours = 0;
      }
      else if (gloAlarmHours >= 3 && gloAlarmHours2 >= 2){
        gloAlarmHours = 0;
      }
      else {
        gloAlarmHours++;
      }
    }
    else if (gloAlarmDigitSelected == 3) {
      if (gloAlarmHours2 >= 2) {
        gloAlarmHours2 = 0;
      }
      else {
        gloAlarmHours2++;
        
        if (gloAlarmHours2 == 2 && gloAlarmHours >= 4) {
          gloAlarmHours = 3;
        }
      }
    }
    
    gloJoyHeldMillis = gloTimeMillis;
    gloJoyStickState = 1;
  }
  // Else if stick is in middle or a second has passed, reset gloJoyStickState to 0 to allow new input on next cycle.
  else if ((gloJoyStickValX > 250 && gloJoyStickValX < 750 && gloJoyStickValY > 250 && gloJoyStickValY < 750 || gloTimeMillis - gloJoyHeldMillis > 1000) && gloJoyStickState == 1) {
    gloJoyStickState = 0;
    gloJoyHeldMillis = gloTimeMillis;
  }
  // Else if no input has been detected for 10 seconds, change clockmode back to 0 to resume displaying time and writes alarm to memory.
  else if (gloTimeMillis - gloJoyHeldMillis > 27000) {
      gloClockMode = 0;
      Serial.println("Changing to Clock mode 0, displaying time..."); // Print debug text.
      MethodWriteAlarmToMemory(); // Run method that writes alarm to memory
  }
}

// Method which writes current alarm to memory
void MethodWriteAlarmToMemory() {
  Serial.println("Alarm time set to: " + String(gloAlarmHours2) + String(gloAlarmHours) + ":" + String(gloAlarmMinutes2) + String(gloAlarmMinutes)); // Print set alarm.
  EEPROM.write(0, gloAlarmMinutes); // Write int alarm minutes to memory.
  EEPROM.write(1, gloAlarmMinutes2); // Write minutes2 to memory.
  EEPROM.write(2, gloAlarmHours); // Write hour to memory.
  EEPROM.write(3, gloAlarmHours2); // Write hour2 to memory.
}

// Method which checks for joystick button press, if it is being held it method 'MethodTmrChangeModeShoo' that checks how long it is held for.
void MethodJoyPress() {
  gloJoyPressVal = digitalRead(diJoyPress); // Read joystick value
  delay(10); // Delay
  gloJoyPressVal2 = digitalRead(diJoyPress); // Read it again.

  //Check whether results were same(debounce function) and run timer method, which will start a 2 second timer, if it is held for 2 seconds change clockMode.
  if (gloJoyPressVal == gloJoyPressVal2 && gloJoyPressVal == 0) {
    MethodTmrChangeModeShoo(); // Run method to switch alarm mode on joystick button held for 2 seconds.
    gloJoyButtonState = 1; // Set button state to 1, which makes sure timer will not be reset.
  }
  // Else reset button state to 0, which will make sure on next run of methodTmrChange timer will be reset.
  else if (gloJoyButtonState == 1) {
    gloJoyButtonState = 0;
  }
}

// Method that will check if joystick button is being held, if it's held for 2 seconds it will perform logic change mode from regular clock display to changing alarm mode and vice versa.
void MethodTmrChangeModeShoo() {
  gloTimeMillis = millis(); // Get time.

  // Resets timer to current time if button state is 0.
  if (gloJoyButtonState == 0) {
    gloJoyPressMillis = gloTimeMillis;
  }

  // If two seconds have passed since button was first pressed, switch clock mode.
  if (gloTimeMillis - gloJoyPressMillis >= 2000) {
    // Change to clock mode "change alarm".
    if (gloClockMode == 0) {
      gloClockMode = 1;
      Serial.println("Changing to Clock mode 1, changing alarm mode..."); // Print debug text.
      gloJoyHeldMillis = gloTimeMillis; // Gets current time for joy held value.
      gloPrevAlarmDigitTimeMillis = gloTimeMillis; // Gets current time for alarm digit blink.
    }
    else if (gloClockMode == 1) {
      gloClockMode = 0;
      Serial.println("Changing to Clock mode 0, displaying time..."); // Print debug text.
      MethodWriteAlarmToMemory(); // Run method that writes alarm to memory
    }

    lcd.clear(); // Clear lcd.
    gloJoyPressMillis = gloTimeMillis; // Resets timer back to 0 seconds, so that will switch again after 2 seconds.
  }
}

// Method which will keep track of time(somewhat accurately).
void MethodTmrTick()
{
  gloTimeMillis = millis(); // Get current time.

  // If 1000 milliseconds have passed, a second has passed, so add 1000 millis to previous time and count a second upwards.
  while (gloTimeMillis - gloPrevTimeMillis >= 1000) {
    gloCountSeconds++; // Count seconds upwards.
    gloPrevTimeMillis = gloPrevTimeMillis + 1000;
    
    // If seconds have reached 60, a minute has passed.
    if (gloCountSeconds >= 60) {
      gloCountMinutes++; // Count minutes upwards.
      gloCountSeconds = 0; // Reset seconds to 0.
        
      // If minutes have reached 60, a second has passed.
      if(gloCountMinutes >= 60)
      {
        gloCountHours++; // Count hours upwards.
        gloCountMinutes = 0; // Reset minutes to 0.
        
        // If hours have reached 24, a day has passed so reset to 0.
        if (gloCountHours >= 24)
        {
          gloCountHours = 0; // Reset hours to 0.
        }
      }

      // If alarm is toggled on and it is currently displaying time, perform logic to check if alarm should fire.
      if (gloDoAlarm == 1 && gloClockMode == 0) {
        Serial.println("Performing alarm check...");
        MethodAlarmCheck();
      }
    }
  }
}

// Method which displays current time.
void MethodShowTime() {
  String locCountHours = String(gloCountHours); // Convert hour int to local string.
  String locCountMinutes = String(gloCountMinutes); // Convert minutes.

  // If string length is 1, add a 0 in front of string, otherwise displaying will go wrong.
  if(locCountHours.length() == 1)
  {
    locCountHours = "0" + locCountHours;
  }
  if(locCountMinutes.length() == 1)
  {
    locCountMinutes = "0" + locCountMinutes;
  }
  
  String locTime = locCountHours + ":" + locCountMinutes; // Format string properly.

  // If alarm is toggled on, add a '.' to display as user feedback.
  if (gloDoAlarm == 1) {
    locTime = locTime + "."; // Add dot to locTime.
  }
  else {
    locTime = locTime + " "; // Add empty space to erase previous '.'.
  }
  
  MethodWriteToLcd(0, 0, locTime); // Write time to display.
}

// Method that prints a string to specific place on Lcd, where x val is place on row, y is colomn and lcdString is string to be printed.
void MethodWriteToLcd(int locValX, int locValY, String locLcdString)
{
  lcd.setCursor(locValX, locValY); // Set lcd to given values.
  
  int locStringLength = locLcdString.length(); // Get length of string.

  // While there is more text left to display in string, print the next character to lcd.
  for(int locStringIndex = 0; locStringIndex < locStringLength; locStringIndex++)
  {
    // If lcd is at end of line, jump to next line.
    if (locValX > 19)
    {
      locValY++;// Jump to next line.

      // If next line is outside of lcd 4 lines, jump back to line 0.
      if(locValY > 3) {
        locValY = 0;
      }

      locValX = 0; // Set val x to beginning of line.
      lcd.setCursor(locValX, locValY); // Set cursor to x an y vals.
    }
    
    lcd.print(locLcdString[locStringIndex]); // Print current character.
    locValX++; // Go to next character.
  }
}
