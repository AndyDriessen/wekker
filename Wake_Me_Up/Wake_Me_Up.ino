#include <Wire.h> // Include wire library, which facilitates serial communication with lcd.
#include <LiquidCrystal_I2C.h> // Include I2C library for lcd screen.

LiquidCrystal_I2C lcd(0x3F, 20, 4); // Initialize new object called lcd.

#define aoJoyXVal A2 // X val on joy
#define aoJoyYVal A3 // Y val on joy
#define diJoyPress 3 // Btn on joy.

int gloTimeMillis = millis(); // Will keep track of how many milliseconds has passed since start of program.
int gloPrevTimeMillis = gloTimeMillis; // Val will keep track of difference between last noted time and current time.
int gloJoyPressMillis = 0; // Will keep track of how long joy button is held.
int gloPrevAlarmTimeMillis = 0; // Timer keeps track of when to blink alarm digit.

int gloCountSeconds = 0; // Seconds.
int gloCountMinutes = 0; // Minutes.
int gloCountHours = 0; // Hours.
int gloClockMode = 0; // Whether clock is or isn't supposed to tick.

int gloAlarmMinutes = 0; // First digit of minute alarm is supposed to go off.
int gloAlarmMinutes2 = 0; // Second digit.
int gloAlarmHours = 0; // First digit of hour alarm is supposed to go off.
int gloAlarmHours2 = 0; // Second digit.
int gloAlarmDigitSelected = 0; // Keeps track of selected digit(one user is changing), from right to left.

int gloJoyPressVal = 0; // Value to keep track if button has been pressed.
int gloJoyPressVal2 = 0; // Value to debounce.
int gloJoyButtonState = 0; // Keeps track of state button is in.
int gloJoyStickValX = 0; // Value to keep track of stick movement.
int gloJoyStickValY = 0; // Value to debounce.
int gloJoyStickState = 0; // Keeps track of state stick is in.

// Initial code at startup.
void setup() {
  Serial.begin(9600); // Open serial console.
  pinMode(diJoyPress, INPUT); // Define joystick button.
  digitalWrite(diJoyPress, HIGH); // Write joystick button high.
  lcd.init(); // initialize the lcd.
  lcd.backlight(); // Turns on backlight, otherwise lcd screen is very dark.
}

// Main loop.
void loop() {
  // If clock is supposed to tick, perform logic to display current time.
  if (gloClockMode == 0) {
    MethodShowTime();
  }
  // Else perform logic to change alarm time.
  else if (gloClockMode == 1) {
    MethodChangeAlarm();
    MethodShowAlarm();
  }

  MethodTmrTick(); // Runs method that makes timer tick.
  MethodJoyPress(); // Run joystick serial monitor output test.
}

void MethodJoyPress() {
  gloJoyPressVal = digitalRead(diJoyPress);
  delay(10);
  gloJoyPressVal2 = digitalRead(diJoyPress);
  if (gloJoyPressVal == gloJoyPressVal2 && gloJoyPressVal == 0) {
    MethodTmrChangeModeShoo(); // Run method to switch to alarm mode.
    gloJoyButtonState = 1; // Set button state to 1, which indicated that the button state has changed.
  }
  else if (gloJoyButtonState == 1) {
    gloJoyButtonState = 0;
  }
}

void MethodShowAlarm() {
  String locAlarmString = "";

  gloTimeMillis = millis(); // Get time.

  // Every half second, blink digit for another half a second, to indicate which character user is changing.
  if (gloTimeMillis - gloPrevAlarmTimeMillis > 400) {
    if (gloAlarmDigitSelected == 0) {
      locAlarmString = "  : " + String(gloAlarmMinutes); // Fill out blank space.
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
    
    if (gloTimeMillis - gloPrevAlarmTimeMillis > 800) {
      gloPrevAlarmTimeMillis = gloTimeMillis; // Update time
    }
  }
  // Else display all character.
  else {
    locAlarmString = String(gloAlarmHours2) + String(gloAlarmHours) + ":" + String(gloAlarmMinutes2) + String(gloAlarmMinutes);
  }
  
  MethodWriteToLcd(0, 0, locAlarmString);
}

void MethodChangeAlarm() {
  gloJoyStickValX = analogRead(aoJoyXVal);
  gloJoyStickValY = analogRead(aoJoyYVal);

  // Check what position the joystick is in and if it has already performed an action in previous cycle(gloJoyStickState). Joystick is right direction.
  if (gloJoyStickValX > 750 && gloJoyStickState == 0) {
    // If there is a digit further to the right, select digit on the right.
    if (gloAlarmDigitSelected > 0) {
      gloAlarmDigitSelected--;
    }
    gloJoyStickState = 1;
  }
  // Joystick is left direction.
  else if (gloJoyStickValX < 250 && gloJoyStickState == 0) {
    // If there is a digit to the left, select digit to the left.
    if (gloAlarmDigitSelected < 3) {
      gloAlarmDigitSelected++;
    }
    gloJoyStickState = 1;
  }
  // Joystick is down direction.
  else if (gloJoyStickValY > 750 && gloJoyStickState == 0) {
    if (gloAlarmDigitSelected == 0) {
      if (gloAlarmMinutes <= 0) {
        gloAlarmMinutes = 9;
      }
      else {
        gloAlarmMinutes--;
      }
    }
    else if (gloAlarmDigitSelected == 1) {
      if (gloAlarmMinutes2 <= 0) {
        gloAlarmMinutes2 = 5;
      }
      else {
        gloAlarmMinutes2--;
      }
    }
    else if (gloAlarmDigitSelected == 2) {
      if (gloAlarmHours <= 0 && gloAlarmHours2 >= 2) {
        gloAlarmHours = 3;
      }
      else if (gloAlarmHours <= 0 && gloAlarmHours2 <= 1){
        gloAlarmHours = 9;
      }
      else {
        gloAlarmHours--;
      }
    }
    else if (gloAlarmDigitSelected == 3) {
      if (gloAlarmHours2 <= 0) {
        gloAlarmHours2 = 2;
        if (gloAlarmHours >= 4) {
          gloAlarmHours = 3;
        }
      }
      else {
        gloAlarmHours2--;
      }
    }
    
    gloJoyStickState = 1;
  }
  // Joystick is down direction.
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
    
    gloJoyStickState = 1;
  }
  // Else if stick is in middle, reset gloJoyStickState to 0 to allow new input on next cycle.
  else if (gloJoyStickValX > 250 && gloJoyStickValX < 750 && gloJoyStickValY > 250 && gloJoyStickValY < 750) {
    gloJoyStickState = 0;
  }

  Serial.println("Vals: " + String(gloJoyStickValY) + " " + String(gloJoyStickValX));
}

// Method that will check if joystick button is being held, if it's held for 2 seconds it will change mode from regular clock display to changing alarm mode and vice versa.
void MethodTmrChangeModeShoo() {
  gloTimeMillis = millis(); // Get time.

  // Resets timer to current time if button state is 0.
  if (gloJoyButtonState == 0) {
    gloJoyPressMillis = gloTimeMillis;
  }

  // If two seconds have passed since button was first pressed, switch clock mode to change alarm
  if (gloTimeMillis - gloJoyPressMillis >= 2000) {
    if (gloClockMode == 0) {
      gloClockMode = 1;
    }
    else if (gloClockMode == 1) {
      gloClockMode = 0;
      gloPrevAlarmTimeMillis = gloTimeMillis; // Gets current time for alarm digit blink.
    }

    lcd.clear();
    gloJoyPressMillis = gloTimeMillis; // Resets timer back to 0 seconds.
  }
}

// Method which will keep track of time(somewhat accurately).
void MethodTmrTick()
{
  gloTimeMillis = millis();

  // If millis have reached 1000, a second has passed.
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
        
        // If millis have reached 24, a second has passed.
        if (gloCountHours >= 24)
        {
          gloCountHours = 0; // Reset hours to 0.
        }
      }
    }
  }
}

void MethodShowTime() {
  String hoursShoo = String(gloCountHours);
  String minutesShoo = String(gloCountMinutes);
  
  if(hoursShoo.length() == 1)
  {
    hoursShoo = "0" + hoursShoo;
  }
  if(minutesShoo.length() == 1)
  {
    minutesShoo = "0" + minutesShoo;
  }
  
  String timeShoo = hoursShoo + ":" + minutesShoo;
  MethodWriteToLcd(0, 0, timeShoo);
}

// Function that prints a string to a given space on Lcd, where x value is place on row, y is colomn and text is string to be printed.
void MethodWriteToLcd(int val_xShoo, int val_yShoo, String textShoo)
{
  lcd.setCursor(val_xShoo, val_yShoo); // Set lcd to given values.
  
  int textLengthShoo = textShoo.length(); // Get length of text.

  // While there is more text left to display in string, print the next character to lcd.
  for(int textIndexShoo = 0; textIndexShoo < textLengthShoo; textIndexShoo++)
  {
    if (val_xShoo > 19)
    {
      val_yShoo++;
      if(val_yShoo > 3) {
        val_yShoo = 0;
      }
      val_xShoo = 0;

      lcd.setCursor(val_xShoo, val_yShoo);
    }
    
    lcd.print(textShoo[textIndexShoo]);
    val_xShoo++;
  }
}
