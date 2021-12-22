
  int sensorValue;
  int stringMiddle = 81;
  int analogMiddle = 340;
  int amplifier = 1;
  int charIndex;
  unsigned int oldChar;
  unsigned int star = '*';
  String s = "|                                                                               |                                                                               |";  


void setup() {
  Serial.begin(250000);
  Serial.println("MicTest2");
}

void loop() {
  // read the input on analog pin 0:
  sensorValue = analogRead(A0);

  charIndex = (sensorValue-analogMiddle) * amplifier + stringMiddle;
  oldChar = s.charAt(charIndex);
  s.setCharAt(charIndex, star);
  Serial.println(s);
  s.setCharAt(charIndex, oldChar);
}
