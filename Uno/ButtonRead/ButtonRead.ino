#define BUTTON 6

byte buttonState = 0;
byte lastButtonState = 0;
byte ledState = LOW;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  digitalWrite(LED_BUILTIN, ledState);
}

void loop() {
  
  buttonState = digitalRead(BUTTON);

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) 
  {
    // if the state has changed, increment the counter
    if (buttonState == LOW) 
    { 
      if (ledState == LOW) {
        ledState = HIGH;   
      }
      else {
        ledState = LOW;
      }
    }

    digitalWrite(LED_BUILTIN, ledState);
    
    // delay a little bit for button debouncing
    delay(50);
  }

  lastButtonState = buttonState;

}
