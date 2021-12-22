#include <PWM.h>                  // PWM Frequency library available at https://code.google.com/archive/p/arduino-pwm-frequency-library/downloads

// VL: Mi bas de la guitare (E2): 82.41 Hz

#define LED_PIN1 10          // pin Arduino pour activer les LED
#define LED_PIN2 3

long frequence1;       // Frequence de pulsation courante (Hz)
long frequence2;       // Frequence de pulsation courante (Hz)
float dutyCycle;      // Duty cycle du PWM de 1 à 100
unsigned int dc;
unsigned long delai;
boolean light_on;

void setup() {

  pinMode(LED_BUILTIN, OUTPUT); // Heart Beat LED
  digitalWrite(LED_BUILTIN, HIGH);

  delai = 500;
  frequence1 = 200;
  frequence2 = 300;
  dutyCycle = 10;
  dc = round(dutyCycle*255/100);
  
  InitTimersSafe();   // initialiser tous les timers sauf 0 qui est utilise pour millis()
}


void loop() {

  delay(delai);
  pwmWrite(LED_PIN1, 255);
  SetPinFrequencySafe(LED_PIN1, frequence1); // ajuster la frequence du PWM
  pwmWrite(LED_PIN1, dc); // ajuster le duty cycle du PWM
  digitalWrite(LED_BUILTIN, LOW);
  
  delay(delai);
  pwmWrite(LED_PIN1, 255);
  SetPinFrequencySafe(LED_PIN1, frequence2); // ajuster la frequence du PWM
  pwmWrite(LED_PIN1, dc); // ajuster le duty cycle du PWM
  digitalWrite(LED_BUILTIN, HIGH);

}
