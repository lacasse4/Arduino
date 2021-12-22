#include "wiring_private.h"

// VL: Mi bas de la guitare (E2): 82.41 Hz

#define FREQUENCE_BASSE   60.0    // frequence basse en Hz (doit etre >= 32)
#define FREQUENCE_HAUTE   200.0   // frequence haute en Hz (doit etre <= 6000)
#define PERIODE           20.0    // periode en seconde
#define DUTY_CYCLE        0.2     // duty cycle en % (ex. mettre 0.2 pour 20%)

#define PRINT1

int compteur;
boolean ledAllume;
boolean monte;
unsigned long delai;
uint8_t topFreqBasse;
uint8_t topFreqHaute;
uint8_t topFreq;
uint8_t dutyFreq;

void setup() {
  #ifdef PRINT
  Serial.begin(250000);
  Serial.println("Sweep9");
  #endif

  pinMode(LED_BUILTIN, OUTPUT); // Heart Beat LED

  topFreqBasse = (uint8_t)round(F_CPU / (2 * FREQUENCE_BASSE * 1024));
  topFreqHaute = (uint8_t)round(F_CPU / (2 * FREQUENCE_HAUTE * 1024));
  
  uint8_t delta = topFreqBasse - topFreqHaute;
  if (delta != 0) delai = 1000 * PERIODE / (2.0 * delta);
  else            delai = 1000;

//  delai = 1000;
  
  topFreq = topFreqBasse;
  dutyFreq = (uint8_t)round(DUTY_CYCLE * topFreq);
  
  compteur = topFreqBasse;
  monte = false;
  ledAllume = false;

  initPWM();
}

void loop() {

  float frequence = F_CPU / (2.0 * topFreq * 1024);
  ledAllume = !ledAllume;
  digitalWrite(LED_BUILTIN, ledAllume ? HIGH : LOW);

  #ifdef PRINT
  Serial.print("FREQ\t");     Serial.print(frequence);
  Serial.print("\tTOP =\t");     Serial.print(topFreq);
  Serial.print("\tDUTY = \t"); Serial.println(dutyFreq);
  #endif

  setTopDuty_PIN10(topFreq, dutyFreq);
  setTopDuty_PIN3 (topFreq, dutyFreq);

  if (monte) {
    compteur++;
    if (compteur > (int)topFreqBasse) {
      topFreq = topFreqBasse - 1;
      monte = false;
    }
    else {
       topFreq = (uint8_t)compteur;
    }
  }
  else {
    compteur--;
    if (compteur < (int)topFreqHaute) {
      topFreq = topFreqHaute + 1;
      monte = true;
    }
    else {
       topFreq = (uint8_t)compteur;
    }
  }

  dutyFreq = (uint8_t)round(DUTY_CYCLE * topFreq); 
  
  delay(delai);
}


/*
 * PWM Library starts here.
 */

#define MAX_FREQ          400.0
#define MIN_FREQ          30.7

boolean setPWM_PIN10(float frequency, float dutyCycle) {
  if (frequency < MIN_FREQ || frequency > MAX_FREQ) return false;
  if (dutyCycle < 0.0 || dutyCycle > 1.0) return false;

  // compute the timer top using a 1024 prescaler
  uint8_t top = (uint8_t)round(F_CPU / (2 * frequency * 1024));

  // convert the duty cycle to an 8 bit integer
  uint8_t duty = (uint8_t)round(dutyCycle * top);

  // set hardware with top and duty cycle
  setTopDuty_PIN10(top, duty);

  // make sure pin 10 is configured as an output
  pinMode(10, OUTPUT);
  return true;
}

void setTopDuty_PIN10(uint8_t top, uint8_t duty) {
  // detect when Timer/Counter reaches the top of counter
  TIFR1 = TIFR1 | _BV(ICF1);    
  while (!(TIFR1 & _BV(ICF1)));
  TIFR1 = TIFR1 | _BV(ICF1);    

  // dectect when Timer/Counter reaches OCR1B downcounting
  TIFR1 = TIFR1 | _BV(OCF1B);    
  while (!(TIFR1 & _BV(OCF1B)));
  TIFR1 = TIFR1 | _BV(OCF1B);

  // it is now safe set top (ICR1) and duty (OCR1B) with risk of a PWM glitch
  ICR1 = (uint16_t)top;
  OCR1B = duty;
}

boolean setPWM_PIN3(float frequency, float dutyCycle) {
  if (frequency < MIN_FREQ || frequency > MAX_FREQ) return false;
  if (dutyCycle < 0.0 || dutyCycle > 1.0) return false;
  
  // compute the timer top using a 1024 prescaler
  uint8_t top = (uint8_t)round(F_CPU / (2 * frequency * 1024));

  // convert the duty cycle to an 8 bit integer
  uint8_t duty = (uint8_t)round(dutyCycle * top);

  setTopDuty_PIN3(top, duty);

  // make sure pin 3 is configured as an output
  pinMode(3, OUTPUT);
  return true;
}

void setTopDuty_PIN3(uint8_t top, uint8_t duty) {
  // detect when Timer/Counter reaches the top of counter
  /* ICF flag does not exist on timer/counter 2
  TIFR1 = TIFR1 | _BV(ICF1);    
  while (!(TIFR1 & _BV(ICF1)));
  TIFR1 = TIFR1 | _BV(ICF1);  
  */  

  // dectect when Timer/Counter reaches OCR2B downcounting
  TIFR2 = TIFR2 | _BV(OCF2B);    
  while (!(TIFR2 & _BV(OCF2B)));
  TIFR2 = TIFR2 | _BV(OCF2B);

  // it is now safe set top (ICR2B) and duty (OCR2B) with risk of a PWM glitch
  OCR2A = top;
  OCR2B = duty;
}

void initPWM() {
  initPWM1_PIN10();
  initPWM2_PIN3();
}

void initPWM1_PIN10() {
  // Timer/Counter 1 (TCNT1) is used with Output Compare Register 1B (OCR1B)
  // to implement a PWM on pin 10 of the Arduino Uno.
  // TCNT1 is used as an 8 bit timer only, to make it work like TCNT2
  // The prescaler is set at 1024

  // set the waveform generation mode
  uint8_t wgm = 8;
  TCCR1A = (TCCR1A & B11111100) | (wgm & 3);
  TCCR1B = (TCCR1B & B11100111) | ((wgm & 12) << 1);

  // set prescaler to 1024
  TCCR1B = (TCCR1B & ~7) | 5;

  // set frequency to MAX_FREQ and duty cycle to 10% at startup
  uint16_t top = (uint16_t)round(F_CPU / (2 * (uint32_t)(MAX_FREQ) * 1024));
  uint8_t duty = (uint8_t)round(0.1 * top);
  ICR1 = top;
  OCR1B = duty;

  // clear ICF1 and OCF1B flags
  TIFR1 = TIFR1 | _BV(ICF1);    
  TIFR1 = TIFR1 | _BV(OCF1B);

  // set pin 10 as the PWM output
  sbi(TCCR1A, COM1B1);

  // set pin 10 DDR as an output
  pinMode(10, OUTPUT);
}

void initPWM2_PIN3() {
  // Timer/Counter 2 (TCNT2) is used with Output Compare Register 2B (OCR2B)
  // to implement a PWM on pin 3 of the Arduino Uno.
  // TCNT2 is a true 8 bit timer only
  // The prescaler is set at 1024

  //setting the waveform generation mode
  uint8_t wgm = 5;
  TCCR2A = (TCCR2A & B11111100) | (wgm & 3);
  TCCR2B = (TCCR2B & B11110111) | ((wgm & 12) << 1);

  // set prescaler to 1024
  TCCR2B = (TCCR2B & ~7) | 7;

  // set frequency to MAX_FREQ and duty cycle to 10% at startup
  uint16_t top = (uint16_t)round(F_CPU / (2 * (uint32_t)(MAX_FREQ) * 1024));
  uint8_t duty = (uint8_t)round(0.1 * top);
  OCR2A = top;
  OCR2B = duty;

  // clear OCF2B flag
  TIFR2 = TIFR2 | _BV(OCF2B);

  // set pin 3 as the PWM output,
  sbi(TCCR2A, COM2B1);

  // set pin 10 DDR as an output
  pinMode(3, OUTPUT);
}
