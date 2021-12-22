#include "wiring_private.h"

// VL: Mi bas de la guitare (E2): 82.41 Hz

#define MAX_FREQ 400.0
#define MIN_FREQ 30.7

float frequence1;       // Frequence de pulsation courante (Hz)
float frequence2;       // Frequence de pulsation courante (Hz)
float dutyCycle;       // Duty cycle du PWM en %
unsigned long delai;
boolean light_on;

void setup() {
  Serial.begin(250000);
  Serial.flush();
  Serial.println("Sweep7");

  pinMode(LED_BUILTIN, OUTPUT); // Heart Beat LED
  digitalWrite(LED_BUILTIN, HIGH);

  delai = 1000;
  frequence1 = 110.0;
  frequence2 = 146.0;
  dutyCycle = 0.10;

  initPWM();
}

void loop() {
  delay(delai);

  SetPWM_PIN10(frequence1, dutyCycle);
  SetPWM_PIN3(frequence1, dutyCycle);
  digitalWrite(LED_BUILTIN, LOW);

  delay(delai);

  SetPWM_PIN10(frequence2, dutyCycle);
  SetPWM_PIN3(frequence2, dutyCycle);
  digitalWrite(LED_BUILTIN, HIGH);  
}

boolean SetPWM_PIN10(float frequency, float dutyCycle) {
  if (frequency < MIN_FREQ || frequency > MAX_FREQ) return false;
  if (dutyCycle < 0.0 || dutyCycle > 1.0) return false;

  // compute the timer top using a 1024 prescaler
  uint16_t top = (uint16_t)round(F_CPU/(2* frequency * 1024));

  // convert the duty cycle to an 8 bit integer
  uint8_t duty = (uint8_t)round(dutyCycle*top);

  // detect when Timer/Counter reaches the top of counter
  TIFR1 = TIFR1 | _BV(ICF1);    
  while (!(TIFR1 & _BV(ICF1)));
  TIFR1 = TIFR1 | _BV(ICF1);    

  // dectect when Timer/Counter reaches OCR1B downcounting
  TIFR1 = TIFR1 | _BV(OCF1B);    
  while (!(TIFR1 & _BV(OCF1B)));
  TIFR1 = TIFR1 | _BV(OCF1B);

  // it is now safe set top (ICR1) and duty (OCR1B) with risk of a PWM glitch
  ICR1 = top;
  OCR1B = duty;

  // make sure pin 10 is configured as an output
  pinMode(10, OUTPUT);
  return true;
}

boolean SetPWM_PIN3(float frequency, float dutyCycle) {
  if (frequency < MIN_FREQ || frequency > MAX_FREQ) return false;
  if (dutyCycle < 0.0 || dutyCycle > 1.0) return false;
  
  // compute the timer top using a 1024 prescaler
  uint16_t top = (uint8_t)round(F_CPU / (2 * frequency * 1024));

  // convert the duty cycle to an 8 bit integer
  uint8_t duty = (uint8_t)round(dutyCycle*top);
  
  // detect when Timer/Counter reaches the top of counter
//  TIFR1 = TIFR1 | _BV(ICF1);    
//  while (!(TIFR1 & _BV(ICF1)));
//  TIFR1 = TIFR1 | _BV(ICF1);    

  // dectect when Timer/Counter reaches OCR2B downcounting
  TIFR2 = TIFR2 | _BV(OCF2B);    
  while (!(TIFR2 & _BV(OCF2B)));
  TIFR2 = TIFR2 | _BV(OCF2B);

  // it is now safe set top (ICR2B) and duty (OCR2B) with risk of a PWM glitch
  OCR2A = top;
  OCR2B = duty;

  // set pin 3 as the PWM output,
  sbi(TCCR2A, COM2B1);

  // make sure pin 3 is configured as an output
  pinMode(3, OUTPUT);
  return true;
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

  // set pin 10 as the PWM output
  sbi(TCCR1A, COM1B1);
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

  // set pin 3 as the PWM output,
  sbi(TCCR2A, COM2B1);
}
