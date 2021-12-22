long frequence = 80;
byte dutyCycle = 10;

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  Serial.flush();
}

void loop() {
  // send data only when you receive data:
  if (Serial.available() > 0) {
    
    // read the incoming byte:
    long freq_tmp = Serial.parseInt();
    byte duty_tmp = Serial.parseInt();

    if (Serial.read() == '\n' && freq_tmp > 0 && duty_tmp > 0) {
      frequence = freq_tmp;
      dutyCycle = duty_tmp;
      Serial.print("Frequence: ");
      Serial.println(frequence);
      Serial.print("Duty cycle: ");
      Serial.println(dutyCycle);
    }
    else {
      Serial.println("*** Erreur de lecture ***");
    }
  }
}
