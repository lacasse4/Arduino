#include <PWM.h>                  // PWM Frequency library available at https://code.google.com/archive/p/arduino-pwm-frequency-library/downloads

// VL: Mi bas de la guitare (E2): 82.41 Hz
//     La bas de la guitare (A2): 110.0 Hz

#define LED_PIN 10        // pin Arduino pour activer les LED
#define MAX_DUTY_CYCLE 30.0 // duty cycle maximum pour proteger le circuit

#define ERR_MAX_DUTY 1
#define ERR_SETPINFREQ 2
#define ERR_LECTURE 3

long frequence = 82;      // Frequence de pulsation des LED
float dutyCycle = 30.0;   // Duty cycle du PWM de 1 à 100
bool succes = false;

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  Serial.flush();
  
  pinMode(LED_BUILTIN, OUTPUT); // Heart Beat LED

  InitTimersSafe();   // initialiser tous les timers sauf 0 qui est utilise pour le time()

  succes = SetPinFrequencySafe(LED_PIN, frequence); // ajuster la frequence du PWM
  if(succes) {
    digitalWrite(LED_BUILTIN, HIGH);    
  }
  
  pwmWrite(LED_PIN, round(dutyCycle*255/100)); // ajuster le duty cycle du PWM

  Serial.println("StrobeLightLecture V1");
  Serial.println("---------------------");
  imprimeStatut();
  Serial.println(">>> Entrez frequence et duty cycle");
  Serial.println();
}

void loop() {

  // "I'm alive" signal
  if (succes) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
  
  // des caracteres sont disponibles sur le port serie
  if (Serial.available() > 0) {
    
    // lire la frequence et le duty cycle
    long freq_tmp = Serial.parseInt();
    float duty_tmp = Serial.parseFloat();

    // verifier que la lecture du port serie s'est bien passee
    if (Serial.read() == '\n' && freq_tmp > 0 && duty_tmp > 0) {
      if (duty_tmp <= MAX_DUTY_CYCLE) {
        succes = SetPinFrequencySafe(LED_PIN, freq_tmp); // ajuster la frequence du PWM
        if (succes) {
          pwmWrite(LED_PIN, round(duty_tmp*255/100)); // ajuster le duty cycle du PWM
          frequence = freq_tmp;
          dutyCycle = duty_tmp;
          imprimeStatut();
        }
        else {
          imprimeErreur(ERR_SETPINFREQ);
          digitalWrite(LED_BUILTIN, LOW);
        }
      }
      else {
        imprimeErreur(ERR_MAX_DUTY);
      }
    }
    else {
      imprimeErreur(ERR_LECTURE);
    }
  }
}

void imprimeStatut() {
  Serial.print("Frequence: ");
  Serial.println(frequence);
  Serial.print("Duty cycle: ");
  Serial.println(dutyCycle);
  Serial.println();
}

void imprimeErreur(int code) {
  switch (code) {
    case ERR_MAX_DUTY:
      Serial.print("*** Erreur: duty cycle maximum ");
      Serial.print(MAX_DUTY_CYCLE);
      Serial.println();
      break;

    case ERR_SETPINFREQ:
      Serial.println("*** Erreur: SetPinFrequencySafe ***");
      break;

    case ERR_LECTURE:
      Serial.println("*** Erreur de lecture ***");
      break;

    default:
      Serial.println("*** Erreur inconnue ***");
      break;
  }
  Serial.println();
}
