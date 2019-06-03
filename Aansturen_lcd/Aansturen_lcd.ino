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

// Define the bit patters for each of our custom chars. These
// are 5 bits wide and 8 dots deep
uint8_t custChar[8][8] = {
  {31, 31, 31, 0, 0, 0, 0, 0},      // Small top line - 0
  {0, 0, 0, 0, 0, 31, 31, 31},      // Small bottom line - 1
  {31, 0, 0, 0, 0, 0, 0, 31},       // Small lines top and bottom -2
  {0, 0, 0, 0, 0, 0,  0, 31},       // Thin bottom line - 3
  {31, 31, 31, 31, 31, 31, 15, 7},  // Left bottom chamfer full - 4
  {28, 30, 31, 31, 31, 31, 31, 31}, // Right top chamfer full -5
  {31, 31, 31, 31, 31, 31, 30, 28}, // Right bottom chamfer full -6
  {7, 15, 31, 31, 31, 31, 31, 31},  // Left top chamfer full -7
};

// Construct numbers 0 through 9.
// 254 is blank space and 255 is completely filled.
uint8_t bigNums[12][6] = {
  {7, 0, 5, 4, 1, 6},         // 0
  {0, 5, 254, 1, 255, 1},     // 1
  {0, 2, 5, 7, 3, 1},         // 2
  {0, 2, 5, 1, 3, 6},         // 3
  {7, 3, 255, 254, 254, 255}, // 4
  {7, 2, 0, 1, 3, 6},         // 5
  {7, 2, 0, 4, 3, 6},         // 6
  {0, 0, 5, 254, 7, 254},     // 7
  {7, 2, 5, 4, 3, 6},         // 8
  {7, 2, 5, 1, 3, 6},         // 9
  {254, 1, 254, 254, 0, 254}, // :
  {254, 254, 254, 254, 1, 1}, // .
};

void setup()
{
  lcd.init(); // initialize the lcd.
  lcd.backlight(); // Turns on backlight, otherwise lcd screen is very dark.

   // Create custom character map (8 characters only!)
  for (int cnt = 0; cnt < sizeof(custChar) / 8; cnt++) {
    lcd.createChar(cnt, custChar[cnt]);
  }

  delay(500);
  
  printBigNum(0, 0, 1);
  printBigNum(1, 4, 1);
  printBigNum(10, 7, 1);
  printBigNum(3, 10, 1);
  printBigNum(4, 14, 1);
  printBigNum(11, 17, 1);
}

// Method that prints large numbers, using the number passed along
void printBigNum(int number, int startCol, int startRow) {

  // Position cursor to requested position (each char takes 3 cols plus a space col)
  lcd.setCursor(startCol, startRow);

  // Each number split over two lines, 3 chars per line. Retrieve character
  // from the main array to make working with it here a bit easier.
  uint8_t thisNumber[6];
  for (int cnt = 0; cnt < 6; cnt++) {
    thisNumber[cnt] = bigNums[number][cnt];
  }

  // First line (top half) of digit
  for (int cnt = 0; cnt < 3; cnt++) {
    lcd.print((char)thisNumber[cnt]);
  }

  // Now position cursor to next line at same start column for digit
  lcd.setCursor(startCol, startRow + 1);

  // 2nd line (bottom half)
  for (int cnt = 3; cnt < 6; cnt++) {
    lcd.print((char)thisNumber[cnt]);
  }
}

void loop()
{
    //MethodTimerTick(); // Perform logic to display current time.
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
