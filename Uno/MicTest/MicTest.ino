void setup() {
  Serial.begin(250000);
  Serial.println("MicTest");
}

void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // print out the value you read:
  Serial.println(sensorValue);
}
