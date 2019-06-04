#include <Wire.h> // Include wire library, which facilitates serial communication with lcd.
#include <LiquidCrystal_I2C.h> // Include I2C library for lcd screen.
#include <EEPROM.h> // Include library that allows us to write data to memory.

LiquidCrystal_I2C lcd(0x3F, 20, 4); // Initialize new object called 'lcd', that will control what is displayed on lcd.

#define aoJoyXVal         A2  // X val on joy.
#define aoJoyYVal         A3  // Y val on joy.
#define diSnoozeBtn       2   // Define snooze button.
#define diJoyPress        3   // Define button on joystick.
#define doBuzzer          5   // Define buzzer to corresponding pin.
#define diBtnToggleAlarm  6   // Define button to toggle alarm.
#define diBtnChangeTone   7   // Define button to change alarm tone to corresponding pin.

unsigned long gloTimeMillis = millis(); // Will keep track of how many milliseconds have passed since start of program.
unsigned long gloPrevTimeMillis = 0;    // Will keep track of difference between last noted time and current time for time keeping purposes.
unsigned long gloJoyHeldMillis = 0;     // Will keep track of how long joystick is being held for.
unsigned long gloJoyPressMillis = 0;    // Will keep track of how long joy button is held in any direction.
unsigned long gloSnoozeTimeShoo = 0;    // Time till snooze is over and alarm will sound again.
unsigned long gloPrevDigitMillis = 0;   // Timer keeps track of when to blink alarm digit when changing alarm time.
unsigned long gloTurnOffSoundShoo = 0;  // Timer keeps track of whether snooze button is being held, if it has been held for three seconds it will turn alarm off.

int gloAlarmMinutes = 0;        // First digit of minute alarm is supposed to go off.
int gloAlarmMinutes2 = 0;       // Second digit.
int gloAlarmHours = 0;          // First digit of hour alarm is supposed to go off.
int gloAlarmHours2 = 0;         // Second digit.
int gloClockTimeMinutes = 0;    // First digit of minute alarm is supposed to go off.
int gloClockTimeMinutes2 = 0;   // Second digit.
int gloClockTimeHours = 0;      // First digit of hour alarm is supposed to go off.
int gloClockTimeHours2 = 0;     // Second digit.

bool gloJoyPressVal = 0;        // Value to keep track if joystick button has been pressed.
bool gloJoyPressVal2 = 0;       // Value to debounce.
int gloBtnJoyState = 0;         // Keeps track of state button is in.
int gloJoyStickValX = 0;        // X value of joystick.
int gloJoyStickValY = 0;        // Y value of joystick.
bool gloJoyStickState = 0;      // Keeps track of state stick is in.

bool gloBtnDoAlarmVal = 0;      // Value to keep track of toggle alarm button press.
bool gloBtnDoAlarmVal2 = 0;     // Debounce value.
bool gloBtnDoAlarmState = 0;    // Keeps track of state button is in.

bool gloBtnChangeToneVal = 0;   // To read button state of change alarm tone.
bool gloBtnChangeToneVal2 = 0;  // Debounce variable.
bool gloBtnChangeToneState = 0; // Records last state of change alarm tone button so only 1 press registers.

bool gloBtnSnoozeVal = 0;       // Value to keep track of snooze btn press.
bool gloBtnSnoozeVal2 = 0;      // Debounce value.
bool gloBtnSnoozeState = 0;     // Keeps track of previous state of snooze button.

bool gloClockMode = 0;          // What mode clock is in(0 is display time, 1 is change time).
bool gloChangeClockOrAlarm = 0; // Variable that keeps track of whether we are changing alarm(0) or clock time(1).
bool gloDoAlarm = 1;            // Whether alarm should go off or not.
bool gloAlarmSnoozed = 0;       // If alarm is currently snoozed.
bool gloSoundAlarm = 0;         // Variable that will make alarm go off.
int gloDigitSelected = 0;       // Keeps track of selected digit(one user is changing), from right to left, so it knows which one not to blink.
int gloAlarmTone = 0;           // Currently configured alarm tone.
int gloFrequency = 0;           // Frequency that will be sent to buzzer.
int gloDoMethodRisingTone = 0;  // Whether rising tone sound should play.
int gloDoMethodFallingTone = 0; // Whethr falling tone sound should play.

// Define the bit patters for each of our custom chars. These are 5 bits wide and 8 dots deep.
// Source for big letters: https://github.com/RalphBacon/LCD_Big_digits/blob/master/LCD_CustChar_Demo_2_With_Serial_Window_Stepthru/LCD_CustChar_Demo_2_With_Serial_Window_Stepthru.ino
// And: https://www.youtube.com/watch?v=8ZsUcUAsL3I
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

// Construct characters 0 through 9, ':' and '.', 254 is blank space and 255 is completely filled.
uint8_t bigNums[13][6] = {
  {7, 0, 5, 4, 1, 6},             // 0
  {0, 5, 254, 1, 255, 1},         // 1
  {0, 2, 5, 7, 3, 1},             // 2
  {0, 2, 5, 1, 3, 6},             // 3
  {7, 3, 255, 254, 254, 255},     // 4
  {7, 2, 0, 1, 3, 6},             // 5
  {7, 2, 0, 4, 3, 6},             // 6
  {0, 0, 5, 254, 7, 254},         // 7
  {7, 2, 5, 4, 3, 6},             // 8
  {7, 2, 5, 1, 3, 6},             // 9
  {254, 1, 254, 254, 0, 254},     // :
  {254, 254, 254, 254, 1, 1},     // .
  {254, 254, 254, 254, 254, 254}, //  
};

// Initial code at startup.
void setup() {
  pinMode(diSnoozeBtn, INPUT); // Initialize snooze button.
  pinMode(diJoyPress, INPUT); // Initialize joystick button.
  pinMode(doBuzzer, OUTPUT); // Set buzzer output.
  pinMode(diBtnToggleAlarm, INPUT); // Initialize toggle alarm button.
  pinMode(diBtnChangeTone, INPUT); // Initialize button to change alarm tone.
  
  Serial.begin(9600); // Open serial console.
  digitalWrite(diJoyPress, HIGH); // Write joystick button high(default, unpressed value is high).
  lcd.init(); // initialize lcd.
  lcd.backlight(); // Turns on backlight, otherwise lcd screen is very dark.
  
  gloAlarmMinutes = EEPROM.read(0); // Get saved alarm values from memory.
  gloAlarmMinutes2 = EEPROM.read(1); // Etc.
  gloAlarmHours = EEPROM.read(2);
  gloAlarmHours2 = EEPROM.read(3);

  // Map the custom characters for custom font(8 characters only!).
  for (int cnt = 0; cnt < sizeof(custChar) / 8; cnt++) {
    lcd.createChar(cnt, custChar[cnt]);
  }

  lcd.setCursor(0, 0); // Go to top of display.

  // Print a line of characters along the top of display, for aesthetics.
  for (int locCount = 0; locCount < 20; locCount++) {
    lcd.print("-");
  }
  
  MethodClockMode0(); // Print current time to screen.

  lcd.setCursor(0, 3); // Go to bottom line of display.

  // Print a line of characters along the bottom of display, for aesthetics.
  for (int locCount = 0; locCount < 20; locCount++) {
    lcd.print("_");
  }
}

// Main loop.
void loop() {
  // If clock mode is 0, run logic to keep track of time, check for alarm and display time on update of digit.
  if (gloClockMode == 0) {
    MethodTmrTick();
  }
  // Else if clock mode is 1, perform logic that allows user to change alarm or clock time.
  else if (gloClockMode == 1) {
    // If ChangeClockOrAlarm is 0, it is in change alarm mode so perform logic to display programmed alarm time and to allow user to change it.
    MethodChangeAlarm();
    MethodShowTime();
  }

  // If alarm isn't currently going off, check for misc inputs.
  if (gloSoundAlarm == 0) {
    MethodJoyPress(); // Runs method that checks whether joystick button has been pressed.
    MethodBtnToggleAlarm(); // Runs timer to toggle alarm on or off.
    MethodChangeAlarmTone(); // Run method to check if user has toggles change alarm tone.
  }
  // If alarm is going off or snoozed, perform logic for snooze button.
  else if (gloSoundAlarm == 1) {
    MethodSoundAlarm();
  }
  
  // If it's supposed to play the rising tone, make it do so.
  if (gloDoMethodRisingTone > 0) {
    MethodRisingTone();
  }
  // Play falling tone if rising tone isn't played and it should play falling tone.
  else if (gloDoMethodFallingTone > 0) {
    MethodFallingTone();
  }
}

// Method which changes the currently configured alarm tone and gives a sound sample.
void MethodChangeAlarmTone() {
  gloBtnChangeToneVal = digitalRead(diBtnChangeTone);
  delay(10);
  gloBtnChangeToneVal2 = digitalRead(diBtnChangeTone);

  // Debounce button and compare it with previous cycle state to make sure it performs logic once each button press.
  if (gloBtnChangeToneVal == gloBtnChangeToneVal2 && gloBtnChangeToneVal != gloBtnChangeToneState)
  {
    gloBtnChangeToneState = gloBtnChangeToneVal;

    // If button has been pressed, go to next tone, reset current sound values and play a sound sample.
    if(gloBtnChangeToneVal == LOW)
    {
      gloAlarmTone++;
      gloFrequency = 0;
      gloDoMethodRisingTone = 0;
      gloDoMethodFallingTone = 0;
      
      if(gloAlarmTone >= 4)
      {
        gloAlarmTone = 0;
      }
    
      Serial.println("Selected tune is: " + String(gloAlarmTone) + ". Sampling sound..."); // Print debug info.
      MethodPlaySound(); // Play user a sample of currently configured sound.
    }
  }
}

// Method that will make current selected tone's sound play.
void MethodPlaySound() {
  Serial.println("Starting alarm sound cycle...");
  
  // Rising tone thrice.
  if (gloAlarmTone == 0) {
    gloFrequency = 0; // Reset the rising tone.
    gloDoMethodRisingTone = 3; // Run this method thrice.
    gloDoMethodFallingTone = 0; // Stop other sound from playing.
  }
  // Falling tone thrice.
  else if(gloAlarmTone == 1) {
    gloFrequency = 1000;
    gloDoMethodRisingTone = 0;
    gloDoMethodFallingTone = 3;
  }
  // Rising tone once, falling tone twice(it is set to three because on first run of falling tone, frequency is 0 so it instantly jumps to finished cycle and resets values, small bug but this negates it.).
  else if (gloAlarmTone == 2) {
    gloFrequency = 0;
    gloDoMethodRisingTone = 1;
    gloDoMethodFallingTone = 3;
  }
  // Rising tone twice, falling tone once.
  else if (gloAlarmTone == 3) {
    gloFrequency = 0;
    gloDoMethodRisingTone = 2;
    gloDoMethodFallingTone = 2;
  }
}

// Method that plays a rising tone.
void MethodRisingTone() {
  // If it hasn't reached stopping frequency(1000), add 100 to frequency and play that sound using MethodBuzzer.
  if (gloFrequency < 1000){
    gloFrequency = gloFrequency + 100;
    MethodBuzzer(5, gloFrequency, 58);
  }
  // Else reset frequency to previous value and subtract one from amount of times this method is supposed to run.
  else {
    gloFrequency = 0;
    gloDoMethodRisingTone--;
  }
}

// Method that plays a falling tone.
void MethodFallingTone() {
  // If it hasn't reached stopping frequency(0), subtract 100 from frequency and play that sound using MethodBuzzer.
  if (gloFrequency > 0){
    gloFrequency = gloFrequency - 100;
    MethodBuzzer(5, gloFrequency, 58);
  }
  // Else reset frequency to previous value and subtract one from amount of times this method is supposed to run.
  else {
    gloFrequency = 1000;
    gloDoMethodFallingTone--;
  }
}

// Method playing sound through buzzer, found on internet, source provided below. Code used from 274 to 290 with minor tweaks.
// https://create.arduino.cc/projecthub/jrance/super-mario-theme-song-w-piezo-buzzer-and-arduino-1cc2e4?ref=search&ref_id=buzzer&offset=5
void MethodBuzzer(int locTargetPin, long locFrequency, long locMLength) {
  digitalWrite(13, HIGH);
  long locDelay = 1000000 / locFrequency / 1; // Was 2, calculate the delay value between transitions. 1 seconds worth of microseconds, divided by the locFrequency, then split in half since there are two phases to each cycle.
  long locNumCycles = locFrequency * locMLength / 1000; // Calculate the number of cycles for proper timing. Multiply locFrequency, which is really cycles per second, by the number of seconds to get the total number of cycles to produce.
  for (long locCurrentCycle = 0; locCurrentCycle < locNumCycles; locCurrentCycle++) {
    digitalWrite(locTargetPin, HIGH); // write the buzzer pin high to push out the diaphram.
    delayMicroseconds(locDelay); // wait for the calculated delay.
    digitalWrite(locTargetPin, LOW); // write the buzzer pin low to pull back the diaphram.
    delayMicroseconds(locDelay); // wait again for the calculated delay.
  }
}

// Method that will handle snooze button logic and sounds alarm if it's supposed to go off.
void MethodSoundAlarm() {
  gloTimeMillis = millis(); // Get time.
  gloBtnSnoozeVal = digitalRead(diSnoozeBtn);
  delay(10);
  gloBtnSnoozeVal2 = digitalRead(diSnoozeBtn);

  // Debounce, check if snooze button is pressed, if it is, snooze alarm. If it is being held for 3 seconds turn alarm off.
  if (gloBtnSnoozeVal == gloBtnSnoozeVal2) {
    // Turn alarm off for 5 minutes on single press and reset sound variables so that alarm sound stops instantly, reset timers.
    if (gloBtnSnoozeState != gloBtnSnoozeVal && gloBtnSnoozeVal == LOW && gloAlarmSnoozed == 0) {
      Serial.println("Snoozed alarm for 5 minutes.");
      gloAlarmSnoozed = 1; // Snooze alarm.
      gloFrequency = 0;
      gloDoMethodRisingTone = 0;
      gloDoMethodFallingTone = 0;
      gloSnoozeTimeShoo = gloTimeMillis;
      gloTurnOffSoundShoo = gloTimeMillis;
    }
    // Else if button has been held for three seconds turn off alarm, reset all sound vars and make sound to notify user it has turned off.
    else if (gloTimeMillis - gloTurnOffSoundShoo > 3000 && gloBtnSnoozeVal == LOW) {
      gloSoundAlarm = 0;
      gloFrequency = 0;
      gloDoMethodRisingTone = 0;
      gloDoMethodFallingTone = 0;
      Serial.println("Alarm turned off.");
      MethodBuzzer(5, 500, 100); // Make noise so that user is notified of alarm turning off.
    }
    // Else if button isn't pressed, reset timer for turning off alarm.
    else if (gloBtnSnoozeVal == HIGH){
        gloTurnOffSoundShoo = gloTimeMillis; // Reset turn off sound time.
    }
    
    gloBtnSnoozeState = gloBtnSnoozeVal; // Update button state.
  }

  // If alarm isn't snoozed and not in the process of playing a sound, run method to play sound of currently configured tone.
  if (gloAlarmSnoozed == 0 && gloDoMethodFallingTone == 0 && gloDoMethodRisingTone == 0) {
    MethodPlaySound();
  }
  // If it is snoozed and 300 seconds(5 min) have passed, sound alarm again.
  else if (gloAlarmSnoozed == 1 && gloTimeMillis - gloSnoozeTimeShoo > 300000) {
    Serial.println("Alarm snooze finished, sounding alarm...");
    gloAlarmSnoozed = 0;
  }
}

// Method that will toggle alarm on or off on button press.
void MethodBtnToggleAlarm() {
  gloBtnDoAlarmVal = digitalRead(diBtnToggleAlarm);
  delay(10);
  gloBtnDoAlarmVal2 = digitalRead(diBtnToggleAlarm);

  // Debounce button, perform logic to toggle alarm on/off.
  if (gloBtnDoAlarmVal == gloBtnDoAlarmVal2 && gloBtnDoAlarmState != gloBtnDoAlarmVal) {
    // Toggle alarm off if it's currently on and button is pressed.
    if (gloDoAlarm == 1 && gloBtnDoAlarmVal == LOW) {
      Serial.println("Toggle alarm button pressed, turning alarm off.");
      gloDoAlarm = 0; // Toggle alarm off.
      MethodPrintBigNum(12, 17, 1); // Print " ".
    }
    // Else toggle alarm on.
    else if (gloDoAlarm == 0 && gloBtnDoAlarmVal == LOW) {
      Serial.println("Toggle alarm button pressed, turning alarm on.");
      gloDoAlarm = 1; // Toggle alarm on.
      MethodPrintBigNum(11, 17, 1); // Print '.'.
    }
    
    gloBtnDoAlarmState = gloBtnDoAlarmVal; // Update button state.
  }
}

// Method which displays the alarm time when user is in process of changing alarm time.
void MethodShowTime() {
  gloTimeMillis = millis(); // Get time.

  // Every 400ms, display only current digit and not others for 400ms. This will create a blinking effect to indicate to user he is changing alarm.
  if (gloTimeMillis - gloPrevDigitMillis > 400) {
    // 
    if (gloDigitSelected == 0) {
      MethodPrintBigNum(12, 0, 1);
      MethodPrintBigNum(12, 4, 1);
      MethodPrintBigNum(12, 10, 1);
    }
    // Etc.
    else if (gloDigitSelected == 1) {
      MethodPrintBigNum(12, 0, 1);
      MethodPrintBigNum(12, 4, 1);
      MethodPrintBigNum(12, 14, 1);
    }
    else if (gloDigitSelected == 2) {
      MethodPrintBigNum(12, 0, 1);
      MethodPrintBigNum(12, 10, 1);
      MethodPrintBigNum(12, 14, 1);
    }
    else if (gloDigitSelected == 3) {
      MethodPrintBigNum(12, 4, 1);
      MethodPrintBigNum(12, 10, 1);
      MethodPrintBigNum(12, 14, 1);
    }

    // If 800 millis have passed, reset time back to 0 so that all digits are shown again.
    if (gloTimeMillis - gloPrevDigitMillis > 800) {
      gloPrevDigitMillis = gloTimeMillis; // Update time
    }
  }
  // Else display all digits.
  else {
    // If current mode is change alarm, display alarm values, otherwise display clock time values.
    if (gloChangeClockOrAlarm == 0) {
      MethodPrintBigNum(gloAlarmHours2, 0, 1);
      MethodPrintBigNum(gloAlarmHours, 4, 1);
      MethodPrintBigNum(gloAlarmMinutes2, 10, 1);
      MethodPrintBigNum(gloAlarmMinutes, 14, 1);
    }
    else if (gloChangeClockOrAlarm == 1) {
      MethodPrintBigNum(gloClockTimeHours2, 0, 1);
      MethodPrintBigNum(gloClockTimeHours, 4, 1);
      MethodPrintBigNum(gloClockTimeMinutes2, 10, 1);
      MethodPrintBigNum(gloClockTimeMinutes, 14, 1);
    }
    
    MethodPrintBigNum(10, 7, 1);
    MethodPrintBigNum(11, 17, 1);
  }
}

// Method which will read joystick analog values, and perform logic to change alarm time as a consequence.
void MethodChangeAlarm() {
  gloJoyStickValX = analogRead(aoJoyXVal); // X val.
  gloJoyStickValY = analogRead(aoJoyYVal); // Y val.
  gloTimeMillis = millis(); // Get time.

  // Check what position the joystick is in and if it has already performed an action in previous cycle(gloJoyStickState). Joystick is right direction.
  if (gloJoyStickValX > 750 && gloJoyStickState == 0) {
    // If there is a digit to the right, select digit on the right.
    if (gloDigitSelected > 0) {
      gloDigitSelected--;
    }

    gloJoyHeldMillis = gloTimeMillis;
    gloJoyStickState = 1;
  }
  // Joystick is left direction, select digit to the left.
  else if (gloJoyStickValX < 250 && gloJoyStickState == 0) {
    // If there is a digit to the left, select digit to the left.
    if (gloDigitSelected < 3) {
      gloDigitSelected++;
    }

    gloJoyHeldMillis = gloTimeMillis;
    gloJoyStickState = 1;
  }
  // Joystick is down direction, subtract 1 of current digit and perform check whether it will go below 0, and if it does, loop it around to max value.
  else if (gloJoyStickValY > 750 && gloJoyStickState == 0) {
    if (gloDigitSelected == 0) {
      if (gloAlarmMinutes <= 0) {
        gloAlarmMinutes = 9; // Max is 9 for digit 0.
      }
      else {
        gloAlarmMinutes--;
      }
    }
    else if (gloDigitSelected == 1) {
      if (gloAlarmMinutes2 <= 0) {
        gloAlarmMinutes2 = 5; // Max is 5 for digit 1.
      }
      else {
        gloAlarmMinutes2--;
      }
    }
    else if (gloDigitSelected == 2) {
      if (gloAlarmHours <= 0 && gloAlarmHours2 >= 2) {
        gloAlarmHours = 3; // Max is 3 if digit 3 is 2.
      }
      else if (gloAlarmHours <= 0 && gloAlarmHours2 <= 1){
        gloAlarmHours = 9; // Max is 9 if digit 3 is < 2
      }
      else {
        gloAlarmHours--;
      }
    }
    else if (gloDigitSelected == 3) {
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
    if (gloDigitSelected == 0) {
      if (gloAlarmMinutes >= 9) {
        gloAlarmMinutes = 0;
      }
      else {
        gloAlarmMinutes++;
      }
    }
    else if (gloDigitSelected == 1) {
      if (gloAlarmMinutes2 >= 5) {
        gloAlarmMinutes2 = 0;
      }
      else {
        gloAlarmMinutes2++;
      }
    }
    else if (gloDigitSelected == 2) {
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
    else if (gloDigitSelected == 3) {
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
      gloPrevTimeMillis = gloTimeMillis; // Reset prev time.
      MethodClockMode0();
  }
}

// Method which displays the newly set time.
void MethodShowClockTime() {
  gloTimeMillis = millis(); // Get time.

  // Every 400ms, display only current digit and not others for 400ms. This will create a blinking effect to indicate to user which digit he is changing.
  if (gloTimeMillis - gloPrevDigitMillis > 400) {
    if (gloDigitSelected == 0) {
      MethodPrintBigNum(12, 0, 1);
      MethodPrintBigNum(12, 4, 1);
      MethodPrintBigNum(10, 7, 1);
      MethodPrintBigNum(12, 10, 1);
      MethodPrintBigNum(gloClockTimeMinutes, 14, 1);
      MethodPrintBigNum(12, 17, 1);
    }
    else if (gloDigitSelected == 1) {
      MethodPrintBigNum(12, 0, 1);
      MethodPrintBigNum(12, 4, 1);
      MethodPrintBigNum(10, 7, 1);
      MethodPrintBigNum(gloClockTimeMinutes2, 10, 1);
      MethodPrintBigNum(12, 14, 1);
      MethodPrintBigNum(12, 17, 1);
    }
    else if (gloDigitSelected == 2) {
      MethodPrintBigNum(12, 0, 1);
      MethodPrintBigNum(gloClockTimeHours, 4, 1);
      MethodPrintBigNum(10, 7, 1);
      MethodPrintBigNum(12, 10, 1);
      MethodPrintBigNum(12, 14, 1);
      MethodPrintBigNum(12, 17, 1);
    }
    else if (gloDigitSelected == 3) {
      MethodPrintBigNum(gloClockTimeHours2, 0, 1);
      MethodPrintBigNum(12, 4, 1);
      MethodPrintBigNum(10, 7, 1);
      MethodPrintBigNum(12, 10, 1);
      MethodPrintBigNum(12, 14, 1);
      MethodPrintBigNum(12, 17, 1);
    }

    // If 800 millis have passed, reset time back to 0 so that all digits are shown again.
    if (gloTimeMillis - gloPrevDigitMillis > 800) {
      gloPrevDigitMillis = gloTimeMillis; // Update time
    }
  }
  // Else display all digits.
  else {
    MethodPrintBigNum(gloClockTimeHours2, 0, 1);
    MethodPrintBigNum(gloClockTimeHours, 4, 1);
    MethodPrintBigNum(10, 7, 1);
    MethodPrintBigNum(gloClockTimeMinutes2, 10, 1);
    MethodPrintBigNum(gloClockTimeMinutes, 14, 1);
    MethodPrintBigNum(12, 17, 1);
  }
}

// Method which will read joystick analog values, and perform logic to change alarm time as a consequence.
void MethodChangeClockTime() {
  gloJoyStickValX = analogRead(aoJoyXVal); // X val.
  gloJoyStickValY = analogRead(aoJoyYVal); // Y val.
  gloTimeMillis = millis(); // Get time.

  // Check what position the joystick is in and if it has already performed an action in previous cycle(gloJoyStickState). Joystick is right direction.
  if (gloJoyStickValX > 750 && gloJoyStickState == 0) {
    // If there is a digit further to the right, select digit on the right.
    if (gloDigitSelected > 0) {
      gloDigitSelected--;
    }

    gloJoyHeldMillis = gloTimeMillis;
    gloJoyStickState = 1;
  }
  // Joystick is left direction, select digit to the left.
  else if (gloJoyStickValX < 250 && gloJoyStickState == 0) {
    // If there is a digit to the left, select digit to the left.
    if (gloDigitSelected < 3) {
      gloDigitSelected++;
    }

    gloJoyHeldMillis = gloTimeMillis;
    gloJoyStickState = 1;
  }
  // Joystick is down direction, subtract 1 of current digit and perform check whether it will go below 0 if a digit were to be subtracted, and if it does, loop it around to max value, otherwise subtract 1.
  else if (gloJoyStickValY > 750 && gloJoyStickState == 0) {
    if (gloDigitSelected == 0) {
      if (gloClockTimeMinutes <= 0) {
        gloClockTimeMinutes = 9; // Max is 9 for digit 0.
      }
      else {
        gloClockTimeMinutes--;
      }
    }
    else if (gloDigitSelected == 1) {
      if (gloClockTimeMinutes2 <= 0) {
        gloClockTimeMinutes2 = 5; // Max is 5 for digit 1.
      }
      else {
        gloClockTimeMinutes2--;
      }
    }
    else if (gloDigitSelected == 2) {
      if (gloClockTimeHours <= 0 && gloClockTimeHours2 >= 2) {
        gloClockTimeHours = 3; // Max is 3 if digit 3 is 2
      }
      else if (gloClockTimeHours <= 0 && gloClockTimeHours2 <= 1){
        gloClockTimeHours = 9; // Max is 9 if digit 3 is < 2
      }
      else {
        gloClockTimeHours--;
      }
    }
    else if (gloDigitSelected == 3) {
      if (gloClockTimeHours2 <= 0) {
        gloClockTimeHours2 = 2; // Max is 2 for digit 3.

        // If digit 2 is above or equal to 4, set it to 3, which is the max value if digit 3 is 2.
        if (gloClockTimeHours >= 4) {
          gloClockTimeHours = 3;
        }
      }
      else {
        gloClockTimeHours2--;
      }
    }
    
    gloJoyHeldMillis = gloTimeMillis;
    gloJoyStickState = 1;
  }
  // Joystick is up direction, add 1 to current digit and perform check whether it will go below max value if 1 were to be added, and if it does, loop it around to min value(0), otherwise add 1, max values are found above.
  else if (gloJoyStickValY < 250 && gloJoyStickState == 0) {
    if (gloDigitSelected == 0) {
      if (gloClockTimeMinutes >= 9) {
        gloClockTimeMinutes = 0;
      }
      else {
        gloClockTimeMinutes++;
      }
    }
    else if (gloDigitSelected == 1) {
      if (gloClockTimeMinutes2 >= 5) {
        gloClockTimeMinutes2 = 0;
      }
      else {
        gloClockTimeMinutes2++;
      }
    }
    else if (gloDigitSelected == 2) {
      if (gloClockTimeHours >= 9 && gloClockTimeHours2 < 2) {
        gloClockTimeHours = 0;
      }
      else if (gloAlarmHours >= 3 && gloClockTimeHours2 >= 2){
        gloClockTimeHours = 0;
      }
      else {
        gloClockTimeHours++;
      }
    }
    else if (gloDigitSelected == 3) {
      if (gloClockTimeHours2 >= 2) {
        gloClockTimeHours2 = 0;
      }
      else {
        gloClockTimeHours2++;
        
        if (gloClockTimeHours2 == 2 && gloClockTimeHours >= 4) {
          gloClockTimeHours = 3;
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
    //gloCountMinutes = (String(gloClockTimeMinutes2) + String(gloClockTimeMinutes)).toInt(); // Combine 2 seperate minute values.
    //gloCountHours = (String(gloClockTimeHours2) + String(gloClockTimeHours)).toInt(); // Combine 2 seperate minute values.
  }
  // Else if no input has been detected for 10 seconds, change clockmode back to 0 to resume displaying time and writes alarm to memory.
  else if (gloTimeMillis - gloJoyHeldMillis > 27000) {
      gloPrevTimeMillis = gloTimeMillis; // Reset prev time.
      MethodClockMode0();
  }
}

// Method which writes current alarm to memory, needs to be method because it's used twice.
void MethodWriteAlarmToMemory() {
  Serial.println("Alarm time set to: " + String(gloAlarmHours2) + String(gloAlarmHours) + ":" + String(gloAlarmMinutes2) + String(gloAlarmMinutes)); // Print set alarm.
  EEPROM.write(0, gloAlarmMinutes); // Write int alarm minutes to memory.
  EEPROM.write(1, gloAlarmMinutes2); // Write minutes2 to memory.
  EEPROM.write(2, gloAlarmHours); // Write hour to memory.
  EEPROM.write(3, gloAlarmHours2); // Write hour2 to memory.
}

// Method which checks for joystick button press, if it is held for 2 seconds it changes alarm mode to either 'change alarm' or 'display time'.
void MethodJoyPress() {
  
  gloJoyPressVal = digitalRead(diJoyPress); // Read joystick value.
  delay(10); // Delay
  gloJoyPressVal2 = digitalRead(diJoyPress); // Read it again.

  // Debounce button, if joystick button is being held for 2 seconds change clockmode.
  if (gloJoyPressVal == gloJoyPressVal2 && gloJoyPressVal == 0) {
    gloTimeMillis = millis(); // Get time.
  
    // Reset joystick press timer to current time if button state is 0.
    if (gloBtnJoyState == 0) {
      gloJoyPressMillis = gloTimeMillis;
      gloBtnJoyState = 1; // Set button state to 1, which makes sure timer will not be reset until button is released.
    }
  
    // If two seconds have passed since button was first pressed, switch clock mode.
    if (gloTimeMillis - gloJoyPressMillis >= 2000) {
      // Change to clock mode "change alarm".
      if (gloClockMode == 0) {
        gloClockMode = 1;
        Serial.println("Changing to Clock mode 1, changing alarm mode..."); // Print debug text.
        gloJoyHeldMillis = gloTimeMillis; // Resets joystick held timer.
        gloPrevDigitMillis = gloTimeMillis; // Gets current time for alarm digit blinker.
        gloChangeClockOrAlarm = 0; // Set changeclock back to 0.
      }
      // Change clock mode to "display time".
      else if (gloClockMode == 1) {
        gloPrevTimeMillis = gloTimeMillis; // Reset prev time.
        MethodClockMode0();
      }

      gloBtnJoyState = 2; // sets button state to 2 so that on release of button, it will not switch ChangeClockOrAlarm.
      gloJoyPressMillis = gloTimeMillis; // Resets timer back to 0 seconds, so that will switch again after 2 seconds.
    }
  }
  // Else reset button state to 0 to register a following press.
  else if (gloBtnJoyState >= 1) {
    // If less then 2 seconds have passed since button was pressed and clockmode is 1 and button state is 1, toggle changeClockOrAlarm mode.
    if (gloTimeMillis - gloJoyPressMillis < 2000 && gloClockMode == 1 && gloBtnJoyState == 1) {
      if (gloChangeClockOrAlarm == 0) {
        gloChangeClockOrAlarm = 1; // Let user change current time.
      }
      else if (gloChangeClockOrAlarm == 1) {
        gloChangeClockOrAlarm = 0; // Let user change alarm time.
      }

      gloPrevDigitMillis = gloTimeMillis; // Reset the flashing animation.
    }
    
    gloBtnJoyState = 0; // Reset button state to 0.
  }
}

// Method which will keep track of time(somewhat accurately).
void MethodTmrTick() {
  gloTimeMillis = millis(); // Get current time.

  // If 60000 milliseconds have passed, a minute has passed, so add 60000 millis to previous time and count a minute upwards.
  while (gloTimeMillis - gloPrevTimeMillis >= 60000) {
    gloPrevTimeMillis = gloPrevTimeMillis + 60000;
    gloClockTimeMinutes++;
      
    // If furthest right digit of minutes has reached 9, set it back to 0 and count minutes2 upwards.
    if(gloClockTimeMinutes >= 9)
    {
      gloClockTimeMinutes2++;
      gloClockTimeMinutes = 0;
      
      // if minutes2 has reached 6, 60 minutes have passed, so set minutes2 back to 0 and add 1 to hours.
      if (gloClockTimeMinutes2 >= 6)
      {
        gloClockTimeHours++;
        gloClockTimeMinutes2 = 0;

        // If hours has reached 10 AND hours2 is less than 2, or hours has reached 4 AND hours2 has reached 2, count hours2 upwards and set hour back to 0.
        if (gloClockTimeHours >= 10 && gloClockTimeHours2 < 2 || gloClockTimeHours >= 4 && gloClockTimeHours2 >= 2) {
          gloClockTimeHours2++;
          gloClockTimeHours = 0;

          // If hours2 has reached 3, reset it back to 0 since a full day has completed
          if (gloClockTimeHours2 >= 3) {
            gloClockTimeHours2 = 0;
          }
          MethodPrintBigNum(gloClockTimeHours2, 0, 1); // Prints hours.
        }
        MethodPrintBigNum(gloClockTimeHours, 4, 1);
      }
      MethodPrintBigNum(gloClockTimeMinutes2, 10, 1); // Print minutes.
    }
    MethodPrintBigNum(gloClockTimeMinutes, 14, 1);
    MethodPrintBigNum(10, 7, 1); // Prints a ':'.
    
    // If alarm is toggled on, add a '.' to display as user feedback.
    if (gloDoAlarm == 1) {
    }
    else {
      MethodPrintBigNum(12, 17, 1); // Print " ".
    }

    // If alarm is toggled on and it is currently displaying time, perform logic to check if alarm should go off.
    if (gloDoAlarm == 1 && gloClockMode == 0) {
      Serial.println("Performing alarm check...");
    
      // If both match, set logic so that alarm sounds.
      if (gloAlarmMinutes == gloClockTimeMinutes && gloAlarmMinutes2 == gloClockTimeMinutes2 && gloAlarmHours == gloClockTimeHours && gloAlarmHours2 == gloClockTimeHours2) {
        Serial.println("Alarm is going to sound.");
        gloSoundAlarm = 1; // Sound alarm.
      }
    }
  }
}

// Method which displays current time and performs logic to redisplay time properly.
void MethodClockMode0() {
    gloClockMode = 0;
    Serial.println("Changing to Clock mode 0, displaying time..."); // Print debug text.
    MethodWriteAlarmToMemory(); // Run method that writes alarm to memory.
    
    MethodPrintBigNum(gloClockTimeHours2, 0, 1); // Prints hours.
    MethodPrintBigNum(gloClockTimeHours, 4, 1);
    MethodPrintBigNum(10, 7, 1); // Prints a ':'.
    MethodPrintBigNum(gloClockTimeMinutes2, 10, 1); // Print minutes.
    MethodPrintBigNum(gloClockTimeMinutes, 14, 1);
    
  // If alarm is toggled on, add a '.' to display as user feedback.
  if (gloDoAlarm == 1) {
    MethodPrintBigNum(11, 17, 1); // Print '.'.
  }
  else {
    MethodPrintBigNum(12, 17, 1); // Print " ".
  }
}

// Method that prints large chars, using the number passed along. For source of code see initialization.
void MethodPrintBigNum(int number, int startCol, int startRow) {
  // Position cursor to requested position (each char takes 3 cols plus a space col).
  lcd.setCursor(startCol, startRow);

  // Each number split over two lines, 3 chars per line. Retrieve character from the main array to make working with it here a bit easier.
  uint8_t thisNumber[6];
  for (int cnt = 0; cnt < 6; cnt++) {
    thisNumber[cnt] = bigNums[number][cnt];
  }

  // Print first line (top half) of digit.
  for (int cnt = 0; cnt < 3; cnt++) {
    lcd.print((char)thisNumber[cnt]);
  }

  // Now position cursor to next line at same start column for digit.
  lcd.setCursor(startCol, startRow + 1);

  // Print 2nd line (bottom half).
  for (int cnt = 3; cnt < 6; cnt++) {
    lcd.print((char)thisNumber[cnt]);
  }
}
