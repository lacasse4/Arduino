int incomingByte = 0; // for incoming serial data
boolean done = false;

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
}

void loop() {
  // send data only when you receive data:
  if (Serial.peek() > 0 && !done) {

    Serial.print("Something happened! ");
    done = true;
  }
}
