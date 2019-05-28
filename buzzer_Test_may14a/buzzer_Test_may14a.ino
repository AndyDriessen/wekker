#define doBuzzer 5 // Define buzzer to corresponding pin.
#define diBtnChangeAlarmTone 7 // Define button to change alarm tone to corresponding pin.

int gloValChangeAlarmTone = 0; // To read button state of change alarm tone.
int gloVal2ChangeAlarmTone = 0; // Debounce variable.
int gloButtonStateChangeAlarmTone = 0; // Records last state of change alarm tone button so only 1 press registers.
int gloAlarmTone = 0; // Currently selected alarm tone.
int gloFrequencyRisingTone = 0; // Frequency.
int gloDoMethodRisingTone = 0; // Whether sound should play.

void setup() {
  pinMode(5, OUTPUT); // Set buzzer output.
  pinMode(diBtnChangeAlarmTone, INPUT); // Button to change alarm tone.

  gloButtonStateChangeAlarmTone = digitalRead(diBtnChangeAlarmTone);
  Serial.begin(9600);
}

void loop() {
  MethodChangeAlarmTone();

  // If it's supposed to play the rising tone, make it do so.
  if (gloDoMethodRisingTone > 0) {
    MethodRisingTone(); // Play rising tone sound.
  }
}

void MethodChangeAlarmTone()
{
  gloValChangeAlarmTone = digitalRead(diBtnChangeAlarmTone);
  delay(10);
  gloVal2ChangeAlarmTone = digitalRead(diBtnChangeAlarmTone);
  if (gloValChangeAlarmTone == gloVal2ChangeAlarmTone)
  {
    if (gloValChangeAlarmTone != gloButtonStateChangeAlarmTone)
    { 
      gloButtonStateChangeAlarmTone = gloValChangeAlarmTone;
      
      if(gloValChangeAlarmTone == LOW)
      {
        gloAlarmTone++;
        
        if(gloAlarmTone >= 3)
        {
          gloAlarmTone = 0;
        }
      
        Serial.println("Selected tune is: " + String(gloAlarmTone) + ". Sampling sound..."); // Print debug info.
        MethodSoundSample(); // run method to give user a sample of sound
      }
    }
  }
}

void MethodSoundSample() {
  if (gloAlarmTone == 0) {
    MethodSoundOne();
  }
  else if (gloAlarmTone == 1)
  {
    MethodSoundTwo();
  }
  else if (gloAlarmTone == 2)
  {
    MethodSoundThree();
  }
}

void MethodSoundOne()
{  
  //int gloValueFrequency = 0;
  MethodRisingTone();
  delay(300);
  buzz(5, 250, 180);
  delay(100);
  buzz(5, 100, 100);
  delay(100);
  buzz(5, 200, 100);
  delay(100);
  buzz(5, 300, 100);
  delay(100);
  buzz(5, 400, 100);
  delay(100);
  buzz(5, 500, 100);
  delay(100);
  buzz(5, 600, 100);
  delay(10);
  buzz(5, 610, 100);
  delay(10);
  buzz(5, 620, 100);
  delay(10);
  buzz(5, 630, 100);
  delay(10);
  buzz(5, 640, 100);
  delay(10);
  buzz(5, 650, 100);
  delay(10);
  buzz(5, 660, 100);
  delay(10);
  buzz(5, 670, 100);
  delay(10);
  buzz(5, 680, 100);
}

void MethodSoundTwo(){
  gloDoMethodRisingTone = 3; // Do method rising tone three times.
}

void MethodSoundThree(){
  gloDoMethodRisingTone = 3; // Do method rising tone three times.
  MethodSoundOne();
}

void MethodRisingTone(){
  if (gloFrequencyRisingTone < 1000){
    gloFrequencyRisingTone = gloFrequencyRisingTone + 100;
    buzz(5, gloFrequencyRisingTone, 58);
  }  
  else {
    gloFrequencyRisingTone = 0;
    gloDoMethodRisingTone--;
  }
}

void buzz(int locTargetPin, long locFrequency, long locMLength) {
  digitalWrite(13, HIGH);
  long locDelay = 1000000 / locFrequency / 1; //wass eerst 2 // calculate the delay gloValChangeAlarmTone between transitions
  //// 1 second's worth of microseconds, divided by the locFrequency, then split in half since
  //// there are two phases to each cycle
  long locNumCycles = locFrequency * locMLength / 1000; // calculate the number of cycles for proper timing
  //// multiply locFrequency, which is really cycles per second, by the number of seconds to
  //// get the total number of cycles to produce
  for (long locCurrentCycle = 0; locCurrentCycle < locNumCycles; locCurrentCycle++) {
    digitalWrite(locTargetPin, HIGH); // write the buzzer pin high to push out the diaphram.
    delayMicroseconds(locDelay); // wait for the calculated delay.
    digitalWrite(locTargetPin, LOW); // write the buzzer pin low to pull back the diaphram.
    delayMicroseconds(locDelay); // wait again for the calculated delay.
  }
}
