#include <Wire.h>
#include <LiquidCrystal_I2C.h>

int timeMillisShoo = millis();
int pastTimeMillisShoo = timeMillisShoo;

int countMillisShoo = 0; // Milliseconds.
int countSecondsShoo = 0; // Seconds.
int countMinutesShoo = 0; // Minutes.
int countHoursShoo = 0; // Hours.
int clockPaused = 0;


LiquidCrystal_I2C lcd(0x3F, 20, 4); // Initialize new object called lcd, that contains

void setup()
{
  lcd.init(); // initialize the lcd.
  lcd.backlight(); // Turns on backlight, otherwise lcd screen is very dark.
}


void loop()
{
  if (clockPaused == 0)
  {
    MethodTimerTick(); // Perform logic to display current time.
  }
}

// Method which will keep track of time(somewhat accurately).
void MethodTimerTick()
{
  int timeMillisShoo = millis();

  // If millis have reached 1000, a second has passed.
  if (timeMillisShoo - pastTimeMillisShoo >= 1000) {
    countSecondsShoo++; // Count seconds upwards.
    pastTimeMillisShoo = timeMillisShoo;
    
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
  String millisShoo = String(timeMillisShoo - pastTimeMillisShoo);
  
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
  while(millisShoo.length() < 3)
  {
    millisShoo = "0" + millisShoo;
  }
  
  String timeShoo = hoursShoo + ":" + minutesShoo + ":" + secondsShoo + "  " + millisShoo;
  lcd.clear(); // Clear lcd.
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
