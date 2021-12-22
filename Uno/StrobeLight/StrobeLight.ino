#include <PWM.h>                  // PWM Frequency library available at https://code.google.com/archive/p/arduino-pwm-frequency-library/downloads

// VL: Mi bas de la guitare (E2): 82.41 Hz

#define FREQUENCE  32     // Frequence de pulsation des LED
#define LED_PIN 10        // pin Arduino pour activer les LED

float dutyCycle = 50.0;   // Duty cycle du PWM de 0 à 100
bool succes[6];


//**********************************************************************************************************************************************************
void setup()
{
  int i;
  for (i = 0; i < 6; i++) succes[i] = false;
  
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  Serial.flush();

  pinMode(LED_BUILTIN, OUTPUT);      // Heart Beat LED
    
  //initialize all timers except for 0, to save time keeping functions
  InitTimers(); 

  //sets the frequency for the specified pin
  succes[0] = SetPinFrequency(6,  FREQUENCE);
  succes[1] = SetPinFrequency(5,  FREQUENCE);
  succes[2] = SetPinFrequency(9,  FREQUENCE);
  succes[3] = SetPinFrequency(10, FREQUENCE);
  succes[4] = SetPinFrequency(11, FREQUENCE);
  succes[5] = SetPinFrequency(3,  FREQUENCE);

  Serial.println("Strobe test");
  for (i = 0; i < 6; i++) {
    Serial.print("succes "); 
    Serial.print(i);
    Serial.print(" = ");
    Serial.print(succes[i]);
    Serial.println();  
  }

  pwmWrite(6,  round(dutyCycle*255/100));   
  pwmWrite(5,  round(dutyCycle*255/100));   
  pwmWrite(9,  round(dutyCycle*255/100));   
  pwmWrite(10, round(dutyCycle*255/100));   
  pwmWrite(11, round(dutyCycle*255/100));   
  pwmWrite(3, round(dutyCycle*255/100));   

}

//**********************************************************************************************************************************************************
void loop()
{ 
  /* "I'm alive" signal */
  bool s = true;
  int i;
  for (i = 0; i < 6; i++) s = s && succes[i];
  if (succes) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
}
