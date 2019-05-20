#include <Wire.h> // Include wire library, which facilitates serial communication with lcd.
#include <LiquidCrystal_I2C.h> // Include I2C library for lcd screen.

LiquidCrystal_I2C lcd(0x3F, 20, 4); // Initialize new object called lcd.

#define aoJoyXValShoo A2 // X val on joy
#define aoJoyYValShoo A3 // Y val on joy
#define diJoyPressShoo 3 // Btn on joy.

int timeMillisShoo = millis(); // Will keep track of how many milliseconds has passed since start of program.
int pastTimeMillisShoo = timeMillisShoo; // Val will keep track of difference between last noted time and current time.
int joyPressMillisShoo = 0; // Will keep track of how long joy button is held.

int countMillisShoo = 0; // Milliseconds.
int countSecondsShoo = 0; // Seconds.
int countMinutesShoo = 0; // Minutes.
int countHoursShoo = 0; // Hours.
int clockPausedShoo = 0; // Whether clock is or isn't supposed to tick.

int joyPressValShoo = 0; // Value to keep track if button has been pressed.
int joyPressVal2Shoo = 0; // Value to debounce.
int joyButtonStateShoo = 0; // Keeps track of state button is in.

// Initial code at startup.
void setup() {
  Serial.begin(9600); // Open serial console.
  pinMode(diJoyPressShoo, INPUT); // Define joystick button.
  digitalWrite(diJoyPressShoo, HIGH); // Write joystick button high.
  lcd.init(); // initialize the lcd.
  lcd.backlight(); // Turns on backlight, otherwise lcd screen is very dark.
}

// Main loop.
void loop() {
  // If clock is supposed to tick, perform logic to display current time.
  if (clockPausedShoo == 0) {
    MethodTmrTick();
  }
  // Else perform logic to change alarm time.
  else if (clockPausedShoo == 1) {
    MethodChangeAlarm();
  }

  MethodJoyPress(); // Run joystick serial monitor output test.
}

void MethodJoyPress() {
  joyPressValShoo = digitalRead(diJoyPressShoo);
  delay(10);
  joyPressVal2Shoo = digitalRead(diJoyPressShoo);
  if (joyPressValShoo == joyPressVal2Shoo && joyPressValShoo == 0) {
    MethodTmrChangeAlarmShoo(); // Run method to switch to alarm mode.
    joyButtonStateShoo = 1; // Set button state to 1, which indicated that the button state has changed.
  }
  else if (joyButtonStateShoo == 1) {
    joyButtonStateShoo = 0;
  }
}

void MethodTmrChangeAlarmShoo() {
  timeMillisShoo = millis(); // Get time.

  // Resets joyPress to current time is button state is 0.
  if (joyButtonStateShoo == 0) {
    joyPressMillisShoo = timeMillisShoo;
  }

  if (timeMillisShoo - joyPressMillisShoo >= 3000) {
    if (clockPausedShoo == 0) {
      clockPausedShoo = 1;
    }
    else if (clockPausedShoo == 1) {
      clockPausedShoo = 0;
    }

    lcd.clear();
    joyPressMillisShoo = timeMillisShoo;
    MethodWriteToLcd(0, 1, "Changed clockPaused to: " + String(clockPausedShoo));
  }
}

// Method which will keep track of time(somewhat accurately).
void MethodTmrTick()
{
  timeMillisShoo = millis();

  // If millis have reached 1000, a second has passed.
  while (timeMillisShoo - pastTimeMillisShoo >= 1000) {
    countSecondsShoo++; // Count seconds upwards.
    pastTimeMillisShoo = pastTimeMillisShoo + 1000;
    
    // If seconds have reached 60, a minute has passed.
    if (countSecondsShoo >= 60) {
      countMinutesShoo++; // Count minutes upwards.
      countSecondsShoo = 0; // Reset seconds to 0.
      
      // If minutes have reached 60, a second has passed.
      if(countMinutesShoo >= 60)
      {
        countHoursShoo++; // Count hours upwards.
        countMinutesShoo = 0; // Reset minutes to 0.
        
        // If millis have reached 24, a second has passed.
        if (countHoursShoo >= 24)
        {
          countHoursShoo = 0; // Reset hours to 0.
        }
      }
    }
  }

  String hoursShoo = String(countHoursShoo);
  String minutesShoo = String(countMinutesShoo);
  String secondsShoo = String(countSecondsShoo);
  
  if(hoursShoo.length() == 1)
  {
    hoursShoo = "0" + hoursShoo;
  }
  if(minutesShoo.length() == 1)
  {
    minutesShoo = "0" + minutesShoo;
  }
  if(secondsShoo.length() == 1)
  {
    secondsShoo = "0" + secondsShoo;
  }
  
  String timeShoo = hoursShoo + ":" + minutesShoo;
  MethodWriteToLcd(0, 0, timeShoo);
  timeShoo = hoursShoo + ":" + minutesShoo + ":" + secondsShoo;
  MethodWriteToLcd(0, 2, timeShoo);
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
