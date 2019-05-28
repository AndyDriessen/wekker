#define buzzerPin 3
int gloSwitchPinOne = 4;
int gloSwitchPinTwo = 5;
int val;
int val2;
int buttonStateOne;
int buttonStateTwo;
int gloCounter = 0;
int gloAlarmTune = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(3, OUTPUT);//buzzer
  pinMode(gloSwitchPinOne, INPUT);//knop
  pinMode(gloSwitchPinTwo, INPUT);//knop

  buttonStateTwo = digitalRead(gloSwitchPinTwo);
  buttonStateOne = digitalRead(gloSwitchPinOne);
  Serial.begin(9600);
}

void loop() {// put your main code here, to run repeatedly:
  buttonAdri();
}


void buttonAdri()
{
  val = digitalRead(gloSwitchPinOne);
  delay(10);
  val2 = digitalRead(gloSwitchPinOne);
  if (val == val2)
  {
    if (val != buttonStateOne)
    { 
      buttonStateOne = val;
      
      if(val == LOW)
      {
        gloAlarmTune++;
        
        if(gloAlarmTune >= 3)
        {
          gloAlarmTune = 0;
        }
      
        Serial.println("Selected tune is: " + String(gloAlarmTune) + ". Sampling sound..."); // Print debug info.
        MethodSoundSample(); // run method to give user a sample of sound
      }
    }
  }
}

void MethodSoundSample() {
  if (gloAlarmTune == 0) {
    MethodSoundOne();
  }
  else if (gloAlarmTune == 1)
  {
    MethodSoundTwo();
  }
  else if (gloAlarmTune == 2)
  {
    MethodSoundThree();
  }
}

void MethodSoundOne()
{  
  //int tellerAdri = 0;
  MethodTeller();
  delay(300);
  buzz(3, 250, 180);
  delay(100);
  buzz(3, 100, 100);
  delay(100);
  buzz(3, 200, 100);
  delay(100);
  buzz(3, 300, 100);
  delay(100);
  buzz(3, 400, 100);
  delay(100);
  buzz(3, 500, 100);
  delay(100);
  buzz(3, 600, 100);
  delay(10);
  buzz(3, 610, 100);
  delay(10);
  buzz(3, 620, 100);
  delay(10);
  buzz(3, 630, 100);
  delay(10);
  buzz(3, 640, 100);
  delay(10);
  buzz(3, 650, 100);
  delay(10);
  buzz(3, 660, 100);
  delay(10);
  buzz(3, 670, 100);
  delay(10);
  buzz(3, 680, 100);
}

void MethodSoundTwo(){
  MethodTeller();
  MethodTeller();
  MethodTeller();
}

void MethodSoundThree(){
  MethodTeller();
  MethodTeller();
  MethodTeller();
  MethodSoundOne();
}

void MethodTeller(){
  for(int tellerAdri = 0; tellerAdri < 1000; tellerAdri = tellerAdri + 100){
    buzz(3, tellerAdri, 58);
    buttonAdri();
  }  
}

void buzz(int targetPin, long frequency, long m_length) {
  digitalWrite(13, HIGH);
  long delayValue = 1000000 / frequency / 1; //wass eerst 2 // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * m_length / 1000; // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to
  //// get the total number of cycles to produce
  for (long i = 0; i < numCycles; i++) { // for the calculated length of time...
    digitalWrite(targetPin, HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin, LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait again or the calculated delay value
  }
}
