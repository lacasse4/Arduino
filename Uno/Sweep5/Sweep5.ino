#include <PWM.h>                  // PWM Frequency library available at https://code.google.com/archive/p/arduino-pwm-frequency-library/downloads
#include "wiring_private.h"

// VL: Mi bas de la guitare (E2): 82.41 Hz

#define LED_PIN1 10          // pin Arduino pour activer les LED
#define LED_PIN2 3
#define LOG

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
  frequence1 = 200;
  frequence2 = 250;
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

  #ifdef LOG 
      Serial.print("\nfrequence1 \t");   Serial.println(frequence1);
      Serial.print("dc         \t");   Serial.println(dc); 
  #endif

  while (!isPWMChangeOK1());
      top = ICR1;
      cnt = TCNT1;
      pct = 100.0*cnt/top;
  mySetFrequency16(frequence1); 
  myPwmWrite_TIMER1B(dc); 
  digitalWrite(LED_BUILTIN, LOW);

//      Serial.print("ICR1 TCNT1 %\t");
//      Serial.print(top); Serial.print("\t");
//      Serial.print(cnt); Serial.print("\t");
//      Serial.print(pct); Serial.print("\t");
//      Serial.println();


  delay(delai);

  #ifdef LOG 
      Serial.print("\nfrequence2 \t");   Serial.println(frequence2);
      Serial.print("dc         \t");   Serial.println(dc); 
  #endif
      top = ICR1;
      cnt = TCNT1;
      pct = 100.0*cnt/top;

  while (!isPWMChangeOK2());
//      top = ICR1;
//      cnt = TCNT1;
//      pct = 100.0*cnt/top;
  mySetFrequency16(frequence2); 
  myPwmWrite_TIMER1B(dc); 
  digitalWrite(LED_BUILTIN, HIGH);  

//      Serial.print("ICR1 TCNT1 %\t");
//      Serial.print(top); Serial.print("\t");
//      Serial.print(cnt); Serial.print("\t");
//      Serial.print(pct); Serial.print("\t");
//      Serial.println();
}

boolean isPWMChangeOK1() {
  uint16_t top;
  uint16_t cnt;
  float pct;
  top = ICR1;
  cnt = TCNT1;
  pct = (float)cnt/top;
  return pct > 0.9 && pct <= 1.0;
}


boolean isPWMChangeOK2() {
  uint16_t top;
  uint16_t cnt;
  float pct;
  top = ICR1;
  cnt = TCNT1;
  pct = (float)cnt/top;
  return pct >= 0.0 && pct < 0.1;
}

void mySetFrequency16(uint32_t f) {

  #ifdef LOG 
      Serial.println("\nmySetFrequency16\n");
  #endif
  
  //find the smallest usable multiplier
  uint16_t multiplier = (int16_t)(F_CPU / (2 * f * UINT16_MAX));

  #ifdef LOG 
      Serial.print("F_CPU      \t");   Serial.println(F_CPU);
      Serial.print("multiplier \t");   Serial.println(multiplier); 
  #endif
  
  uint8_t iterate = 0;
  while(multiplier > pscLst[iterate++]);
  
  multiplier = pscLst[iterate]; //multiplier holds the clock select value, and iterate holds the corresponding CS flag

  #ifdef LOG 
      Serial.print("iterate    \t");   Serial.println(iterate);
      Serial.print("multiplier \t");   Serial.println(multiplier); 
  #endif     
  
  //getting the timer top using the new multiplier
  uint16_t timerTop = (uint16_t)(F_CPU/(2* f * multiplier));

  #ifdef LOG 
      Serial.print("timerTop   \t");   Serial.println(timerTop);
  #endif     
  
          SetTop_16(timerTop);
          SetPrescaler_16((prescaler)iterate);
}


void myPwmWrite_TIMER1B(uint8_t val) {
  #ifdef LOG 
      Serial.println("\nmmyPwmWrite_TIMER1B\n");
  #endif

  
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
      
    #ifdef LOG 
      Serial.print("val           \t");   Serial.println(val);
      Serial.print("regLoc16      \t");   Serial.println(regLoc16); 
      Serial.print("top           \t");   Serial.println(top); 
      Serial.print("(tmp*top)/255 \t");   Serial.println((tmp*top)/255); 
    #endif     
  }   
}
