#include <PWM.h>                  // PWM Frequency library available at https://code.google.com/archive/p/arduino-pwm-frequency-library/downloads
#include "wiring_private.h"

// VL: Mi bas de la guitare (E2): 82.41 Hz

#define LED_PIN1 10          // pin Arduino pour activer les LED
#define LED_PIN2 3
//#define LOG

long frequence1;       // Frequence de pulsation courante (Hz)
long frequence2;       // Frequence de pulsation courante (Hz)
float dutyCycle;      // Duty cycle du PWM de 1 à 100
unsigned int dc;
unsigned long delai;
boolean light_on;

void setup() {

  Serial.begin(250000);
  Serial.flush();
  Serial.println("Sweep5");

  pinMode(LED_BUILTIN, OUTPUT); // Heart Beat LED
  digitalWrite(LED_BUILTIN, HIGH);

  delai = 1000;
  frequence1 = 82;
  frequence2 = 83;
  dutyCycle = 10;
  dc = round(dutyCycle*255/100);
  
  InitTimersSafe();   // initialiser tous les timers sauf 0 qui est utilise pour millis()
}


void loop() {
  uint16_t top;
  uint16_t cnt;
  float pct;
  uint32_t compteur;

  delay(delai);

  while (ICR1 - TCNT1 > 50);
  SetFrequency16_PS8(frequence1); 
  myPwmWrite_TIMER1B(dc); 
  digitalWrite(LED_BUILTIN, LOW);

  delay(delai);

  while (ICR1 - TCNT1 > 50);
  SetFrequency16_PS8(frequence2); 
  myPwmWrite_TIMER1B(dc); 
  digitalWrite(LED_BUILTIN, HIGH);  
}

void SetFrequency16_PS8(uint32_t f) {

  uint16_t multiplier = 8;
  uint8_t iterate = 2;
  
  //getting the timer top using the new multiplier
  uint16_t timerTop = (uint16_t)(F_CPU/(2* f * multiplier));
  
  SetTop_16(timerTop);
  SetPrescaler_16((prescaler)iterate);
}


void myPwmWrite_TIMER1B(uint8_t val) {
  
  pinMode(LED_PIN1, OUTPUT);
  
  //casting "val" to be larger so that the final value (which is the partially
  //the result of multiplying two potentially high value int16s) will not truncate
  uint32_t tmp = val;
  
  if (val == 0)
    digitalWrite(LED_PIN1, LOW);
  else if (val == 255)
    digitalWrite(LED_PIN1, HIGH);
  else
  {
    uint16_t regLoc16 = 0;
    uint16_t regLoc8 = 0;
    
    uint16_t top;

    // start - TIMER1B specific
    sbi(TCCR1A, COM1B1);
    regLoc16 = OCR1B_MEM;
    top = Timer1_GetTop();
    // end - TIMER1B specific
    
    if(regLoc16)
      _SFR_MEM16(regLoc16) = (tmp*top)/255;
    else
      _SFR_MEM8(regLoc8) = (tmp*top)/255;      
  }   
}
