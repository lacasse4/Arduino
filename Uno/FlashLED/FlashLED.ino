void setup() {
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(6, LOW);
  digitalWrite(9, LOW);
  digitalWrite(11, LOW);
  digitalWrite(13, LOW);
  delay(100);
  digitalWrite(6, HIGH);
  digitalWrite(9, HIGH);
  digitalWrite(11, HIGH);
  digitalWrite(13, HIGH);

  delay(500);
}
