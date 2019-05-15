#define aoJoyXValShoo A2
#define aoJoyYValShoo A3
#define diJoyPressShoo 2
 
void setup() {
  pinMode(diJoyPressShoo, INPUT);
  digitalWrite(diJoyPressShoo, HIGH);
  Serial.begin(9600);
}
 
void loop() {
  int xValue = analogRead(aoJoyXValShoo);
  int yValue = analogRead(aoJoyYValShoo);
  int pressedValue = digitalRead(diJoyPressShoo);
 
  //print the values with to plot or view
  Serial.print(pressedValue);
  Serial.print("\t");
  Serial.print(xValue);
  Serial.print("\t");
  Serial.println(yValue);
  delay(500);
}
