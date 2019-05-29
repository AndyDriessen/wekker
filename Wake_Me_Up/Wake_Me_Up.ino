#include <Wire.h> // Include wire library, which facilitates serial communication with lcd.
#include <LiquidCrystal_I2C.h> // Include I2C library for lcd screen.
#include <EEPROM.h> // Include library that allows us to write data to memory.

LiquidCrystal_I2C lcd(0x3F, 20, 4); // Initialize new object called 'lcd', that will control what is displayed on lcd.

#define aoJoyXVal A2 // X val on joy.
#define aoJoyYVal A3 // Y val on joy.
#define diSnoozeBtn 2 // Define snooze button.
#define diJoyPress 3 // Define button on joystick.
#define doBuzzer 5 // Define buzzer to corresponding pin.
#define diBtnToggleAlarm 6 // Define button to toggle alarm.
#define diBtnChangeTone 7 // Define button to change alarm tone to corresponding pin.

unsigned long gloTimeMillis = millis(); // Will keep track of how many milliseconds have passed since start of program.
unsigned long gloPrevTimeMillis = gloTimeMillis; // Will keep track of difference between last noted time and current time.
unsigned long gloJoyHeldMillis = 0; // Will keep track of how long joystick is being held for.
unsigned long gloJoyPressMillis = 0; // Will keep track of how long joy button is held in any direction.
unsigned long gloPrevDigitMillis = 0; // Timer keeps track of when to blink alarm digit when changing alarm time.
unsigned long gloTurnOffSoundShoo = 0; // Timer keeps track of whether snooze button is being held, if it has been held for three seconds it will turn alarm off.
unsigned long gloSnoozeTimeShoo = 0; // Time till snooze is over and alarm will sound again.

int gloCountMinutes = 0; // Minutes.
int gloCountHours = 0; // Hours.
bool gloClockMode = 0; // What mode clock is in(0 is display time, 1 is change alarm).

int gloAlarmMinutes = 0; // First digit of minute alarm is supposed to go off.
int gloAlarmMinutes2 = 0; // Second digit.
int gloAlarmHours = 0; // First digit of hour alarm is supposed to go off.
int gloAlarmHours2 = 0; // Second digit.
int gloAlarmDigitSelected = 0; // Keeps track of selected digit(one user is changing), from right to left, so it knows which one not to blink.

bool gloJoyPressVal = 0; // Value to keep track if joystick button has been pressed.
bool gloJoyPressVal2 = 0; // Value to debounce.
bool gloJoyButtonState = 0; // Keeps track of state button is in.
int gloJoyStickValX = 0; // Value to keep track of stick movement.
int gloJoyStickValY = 0; // Value to debounce.
bool gloJoyStickState = 0; // Keeps track of state stick is in.

bool gloBtnToggleAlarmVal = 0; // Value to keep track of toggle alarm button press.
bool gloBtnToggleAlarmVal2 = 0; // Debounce value.
bool gloBtnToggleAlarmState = 0; // Keeps track of state button is in.

bool gloBtnChangeToneVal = 0; // To read button state of change alarm tone.
bool gloBtnChangeToneVal2 = 0; // Debounce variable.
bool gloBtnChangeToneState = 0; // Records last state of change alarm tone button so only 1 press registers.

bool gloBtnSnoozeVal = 0; // Value to keep track of snooze btn press.
bool gloBtnSnoozeVal2 = 0; // Debounce value.
bool gloBtnSnoozeState = 0; // Keeps track of previous state of snooze button.

bool gloDoAlarm = 1; // Whether alarm should go off or not.
bool gloAlarmSnoozed = 0; // If alarm is currently snoozed.
bool gloSoundAlarm = 0; // Variable that will make alarm go off.
int gloAlarmTone = 0; // Currently configured alarm tone.
int gloFrequency = 0; // Frequency that will be sent to buzzer.
int gloDoMethodRisingTone = 0; // Whether rising tone sound should play.
int gloDoMethodFallingTone = 0; // Whethr falling tone sound should play.

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
  
  //gloBtnChangeToneState = digitalRead(diBtnChangeTone); // Get current alarm state Primed for deletion, suspected useless code.
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
    
    // If alarm is supposed to go off, sound alarm and perform logic for snooze button..
    if (gloSoundAlarm == 1) {
      MethodSoundAlarm();
    }
  }
  // Else if clock mode is 1, perform logic to change alarm time and show alarm on screen.
  else if (gloClockMode == 1) {
    MethodChangeAlarm();
    MethodShowAlarm();
  }
  
  MethodTmrTick(); // Runs method that makes timer tick.

  // If alarm isn't going off, check for misc inputs.
  if (gloSoundAlarm == 0) {
    MethodJoyPress(); // Runs method that checks whether joystick button has been pressed.
    MethodBtnToggleAlarm(); // Runs timer to toggle alarm on or off.
    MethodChangeAlarmTone(); // Run method to check if user has toggles change alarm tone.
  }
  
  // If it's supposed to play the rising tone, make it do so.
  if (gloDoMethodRisingTone > 0) {
    MethodRisingTone(); // Play rising tone sound.
  }
  // Play falling tone if rising tone isn't played.
  else if (gloDoMethodFallingTone > 0) {
    MethodFallingTone(); // Play falling tone sound.
  }
}

// Method which changes the currently configured alarm tone and gives a sample.
void MethodChangeAlarmTone() {
  gloBtnChangeToneVal = digitalRead(diBtnChangeTone);
  delay(10);
  gloBtnChangeToneVal2 = digitalRead(diBtnChangeTone);

  // Debounce button and compare it with previous cycle state to make sure it doesn't perform logic more than once.
  if (gloBtnChangeToneVal == gloBtnChangeToneVal2 && gloBtnChangeToneVal != gloBtnChangeToneState)
  {
    gloBtnChangeToneState = gloBtnChangeToneVal;

    // If button has been pressed, configure tone to next frequency, reset current sound values and play a sample of the sound.
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
  // Rising tone once, falling tone twice(it is set to three because on first run of falling tone, frequency is 0 so it instantly jumps to finished cycle and resets values.).
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
    Serial.println("Finished an alarm sound cycle.");
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
    Serial.println("Finished an alarm sound cycle.");
  }
}

// Method for producing sound out of buzzer, found from sample code on internet, source provided below, code used from 274 to 290 with minor tweaks.
// https://create.arduino.cc/projecthub/jrance/super-mario-theme-song-w-piezo-buzzer-and-arduino-1cc2e4?ref=search&ref_id=buzzer&offset=5
void MethodBuzzer(int locTargetPin, long locFrequency, long locMLength) {
  digitalWrite(13, HIGH);
  long locDelay = 1000000 / locFrequency / 1; // Was 2, calculate the delay gloBtnChangeToneVal between transitions.
  // 1 second's worth of microseconds, divided by the locFrequency, then split in half since.
  // there are two phases to each cycle.
  long locNumCycles = locFrequency * locMLength / 1000; // calculate the number of cycles for proper timing.
  // multiply locFrequency, which is really cycles per second, by the number of seconds to.
  // get the total number of cycles to produce.
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
  gloBtnSnoozeVal = digitalRead(diSnoozeBtn); // Read button values.
  delay(10);
  gloBtnSnoozeVal2 = digitalRead(diSnoozeBtn); // Read it again.

  // Debounce, check if snooze button is pressed, if it is snooze alarm, if it is held for 3 seconds turn alarm off.
  if (gloBtnSnoozeVal == gloBtnSnoozeVal2) {
    // Turn alarm off for 5 minutes on single press and reset sound variables so that alarm sound stops, reset timers.
    if (gloBtnSnoozeState != gloBtnSnoozeVal && gloBtnSnoozeVal == LOW && gloAlarmSnoozed == 0) {
      Serial.println("Snoozed alarm for 5 minutes.");
      gloAlarmSnoozed = 1; // Snooze alarm.
      gloFrequency = 0;
      gloDoMethodRisingTone = 0;
      gloDoMethodFallingTone = 0;
      gloSnoozeTimeShoo = gloTimeMillis;
      gloTurnOffSoundShoo = gloTimeMillis;
    }
    // If button has been held for three seconds turn off alarm, reset all sound vars and make sound to notify user it has turned off.
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
  // If it is snoozed and 300 seconds have passed, sound alarm again.
  else if (gloAlarmSnoozed == 1 && gloTimeMillis - gloSnoozeTimeShoo > 300000) {
    Serial.println("Alarm snooze finished, sounding alarm...");
    gloAlarmSnoozed = 0;
  }
}

// Method that will toggle alarm on or off on corresponding button press.
void MethodBtnToggleAlarm() {
  gloBtnToggleAlarmVal = digitalRead(diBtnToggleAlarm); // Read btn value.
  delay(10); // Delay
  gloBtnToggleAlarmVal2 = digitalRead(diBtnToggleAlarm); // Read it again.

  // Debounce button, perform logic to toggle alarm on/off.
  if (gloBtnToggleAlarmVal == gloBtnToggleAlarmVal2 && gloBtnToggleAlarmState != gloBtnToggleAlarmVal) {
    // Toggle alarm off if if it's currently on, buttonstate is different from previous cycle and button is pressed.
    if (gloDoAlarm == 1 && gloBtnToggleAlarmVal == LOW) {
      Serial.println("Toggle alarm button pressed, turning alarm off.");
      gloDoAlarm = 0; // Toggle alarm off.
    }
    // Else toggle alarm on.
    else if (gloDoAlarm == 0 && gloBtnToggleAlarmVal == LOW) {
      Serial.println("Toggle alarm button pressed, turning alarm on.");
      gloDoAlarm = 1; // Toggle alarm on.
    }
    
    gloBtnToggleAlarmState = gloBtnToggleAlarmVal; // Update button state.
  }
}

// Method which displays the alarm time.
void MethodShowAlarm() {
  String locAlarmString = ""; // String which will be displayed.

  gloTimeMillis = millis(); // Get time.

  // Every 400ms, display only current digit and not others for 400ms. This will create a blinking effect to indicate to user he is changing alarm.
  if (gloTimeMillis - gloPrevDigitMillis > 400) {
    if (gloAlarmDigitSelected == 0) {
      locAlarmString = "  : " + String(gloAlarmMinutes); // Only display furthest right digit.
    }
    else if (gloAlarmDigitSelected == 1) {
      locAlarmString = "  :" + String(gloAlarmMinutes2) + " "; // Etc.
    }
    else if (gloAlarmDigitSelected == 2) {
      locAlarmString = " " + String(gloAlarmHours) + ":  ";
    }
    else if (gloAlarmDigitSelected == 3) {
      locAlarmString = String(gloAlarmHours2) + " :  ";
    }

    // If 800 millis have passed, reset time back to 0 so that all digits are shown again.
    if (gloTimeMillis - gloPrevDigitMillis > 800) {
      gloPrevDigitMillis = gloTimeMillis; // Update time
    }
  }
  // Else display all digits.
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
    if (gloJoyButtonState == 0) {
      gloJoyPressMillis = gloTimeMillis;
    }
  
    // If two seconds have passed since button was first pressed, switch clock mode.
    if (gloTimeMillis - gloJoyPressMillis >= 2000) {
      // Change to clock mode "change alarm".
      if (gloClockMode == 0) {
        gloClockMode = 1;
        Serial.println("Changing to Clock mode 1, changing alarm mode..."); // Print debug text.
        gloJoyHeldMillis = gloTimeMillis; // Resets joystick held timer.
        gloPrevDigitMillis = gloTimeMillis; // Gets current time for alarm digit blinker.
      }
      // Change clock mode to "display time".
      else if (gloClockMode == 1) {
        gloClockMode = 0;
        Serial.println("Changing to Clock mode 0, displaying time..."); // Print debug text.
        MethodWriteAlarmToMemory(); // Run method that writes alarm newly set alarm to memory.
      }
  
      lcd.clear(); // Clear lcd for new displayed text.
      gloJoyPressMillis = gloTimeMillis; // Resets timer back to 0 seconds, so that will switch again after 2 seconds.
    }
    
    gloJoyButtonState = 1; // Set button state to 1, which makes sure timer will not be reset.
  }
  // Else reset button state to 0 to register a following press.
  else if (gloJoyButtonState == 1) {
    gloJoyButtonState = 0;
  }
}

// Method which will keep track of time(somewhat accurately).
void MethodTmrTick() {
  gloTimeMillis = millis(); // Get current time.

  // If 60000 milliseconds have passed, a minute has passed, so add 60000 millis to previous time and count a minute upwards.
  while (gloTimeMillis - gloPrevTimeMillis >= 60000) {
    gloPrevTimeMillis = gloPrevTimeMillis + 60000;
    gloCountMinutes++;
      
    // If minutes have reached 60, an hour has passed, so count hour upward and reset minutes to 0.
    if(gloCountMinutes >= 60)
    {
      gloCountHours++;
      gloCountMinutes = 0;
      
      // If hours have reached 24, a day has passed so reset hours to 0.
      if (gloCountHours >= 24)
      {
        gloCountHours = 0;
      }
    }

    // If alarm is toggled on and it is currently displaying time, perform logic to check if alarm should go off.
    if (gloDoAlarm == 1 && gloClockMode == 0 && gloClockMode == 0) {
      Serial.println("Performing alarm check...");
      int locAlarmMinutes = (String(gloAlarmMinutes2) + String(gloAlarmMinutes)).toInt(); // Combine two seperate alarm minute values.
      int locAlarmHours = (String(gloAlarmHours2) + String(gloAlarmHours)).toInt(); // Combine hour values.
    
      // If both match, set logic so that alarm sounds.
      if (locAlarmMinutes == gloCountMinutes && locAlarmHours == gloCountHours) {
        Serial.println("Alarm is going to sound.");
        gloSoundAlarm = 1; // Sound alarm.
        //gloTurnOffSoundShoo = gloTimeMillis; // Reset time for turning off sound i suspect it is dead code.
      }
    }
  }
}

// Method which displays current time.
void MethodShowTime() {
  String locCountHours = String(gloCountHours); // Convert hour int to local string.
  String locCountMinutes = String(gloCountMinutes); // Convert minutes.

  // If string length is 1, add a 0 in front of string, otherwise displaying will go wrong and display #:# instead of ##:##.
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
  
  MethodWriteToLcd(0, 0, locTime); // Write time string to display.
}

// Method that prints a string to specific place on Lcd, where x val is place on row, y is colomn and lcdString is string to be printed.
void MethodWriteToLcd(int locValX, int locValY, String locLcdString) {
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
      lcd.setCursor(locValX, locValY); // Set cursor to x an y vals.s
    }
    
    lcd.print(locLcdString[locStringIndex]); // Print current character.
    locValX++; // Go to next character.
  }
}
