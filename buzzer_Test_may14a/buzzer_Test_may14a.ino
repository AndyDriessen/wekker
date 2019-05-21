#define buzzerPinAdri 3
int switchPinAdri = 4;
int val;
int val2;
int buttonState;
int counterAdri = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(3, OUTPUT);//buzzer
  pinMode(switchPinAdri, INPUT);//knop
  
  buttonState = digitalRead(switchPinAdri);
  Serial.begin(9600);
}

void loop() {// put your main code here, to run repeatedly:
  buttonAdri();
}

void MethodSoundOneAdri()
{  
  //int tellerAdri = 0;
  MethodTellerAdri();
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

void buttonAdri()
{
  val = digitalRead(switchPinAdri);
  delay(10);
  val2 = digitalRead(switchPinAdri);
  if (val == val2)
  {
    if (val != buttonState)
      {
        while(val == LOW)
          {
            counterAdri++;
            Serial.println(counterAdri);
            //MethodTellerAdri();
            if(counterAdri == 10)
            {
               Serial.print("10");
               MethodSoundOneAdri();
               delay(4000);
               MethodSoundOneAdri();
               delay(4000);
               MethodSoundOneAdri();
            }
            else if(counterAdri == 20)
            {
              Serial.print("20");
              MethodTellerAdri();
              MethodTellerAdri();
              MethodTellerAdri();
            }
         }
      }
    buttonState = val;
  }
}


void MethodTellerAdri(){
  for(int tellerAdri = 0; tellerAdri < 1000; tellerAdri = tellerAdri + 100){
    buzz(3, tellerAdri, 58);
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
