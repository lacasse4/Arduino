#include <PWM.h>                  // PWM Frequency library available at https://code.google.com/archive/p/arduino-pwm-frequency-library/downloads

// VL: Mi bas de la guitare (E2): 82.41 Hz

#define LED_PIN1 10          // pin Arduino pour activer les LED
#define LED_PIN2 3
#define MAX_DUTY_CYCLE 50.0 // duty cycle maximum pour proteger le circuit (%)
#define MIN_FREQ 32         // frequence minimum du PWM (Hz)
#define MAX_FREQ 62500      // frequence maximum du PWM (hz)
#define MIN_PERIODE 1       // periode minimum (sec)
#define MAX_PERIODE 600     // periode maximum (sec)

long freq_bas;        // Frequence basse de pulsation des LED (Hz)
long freq_haut;       // Frequence haute de pulsation de LED (Hz)
long frequence;       // Frequence de pulsation courante (Hz)
long freq_prec;       // Frequence de pulsation precedente
float dutyCycle;      // Duty cycle du PWM de 1 à 100
int periode;          // Periode du sweep (sec)
int periode_ms;       // Periode du sweep en ms.
int compteur_ms;      // Compteur de ms pour parcourir la periode
int stay_target_ms;          // Compteur pour rester en fin de course.
int stay_compteur_ms;

typedef enum { UP, STAY_UP, DOWN, STAY_DOWN} t_statut;
t_statut statut;      // pour le heart beat

unsigned long this_millis;
unsigned long last_millis = 0;
bool succes1;          // true si le PMW est en fonction
bool succes2;          // true si le PMW est en fonction

void setup() {

  pinMode(LED_BUILTIN, OUTPUT); // Heart Beat LED

  InitTimersSafe();   // initialiser tous les timers sauf 0 qui est utilise pour millis()
  
  setParam(
    1,         // periode du sweep (sec)
    100,       // frequence basse (Hz)
    300,       // frequence haute (Hz)
    50);       // duty cycle (%)
}


void loop() {

  static t_statut old_statut = UP;
  if (statut != old_statut) {
    old_statut = statut;
    if (statut == UP)        digitalWrite(LED_BUILTIN, HIGH);
    else if (statut == DOWN) digitalWrite(LED_BUILTIN, LOW);
  }

  this_millis = millis();
  if (last_millis != this_millis) {
    last_millis = this_millis;
    switch (statut) {
      case UP:
      compteur_ms++;
      if (compteur_ms >= periode_ms) {
        statut = STAY_UP;
      } 
      break;

      case STAY_UP:
      stay_compteur_ms++;
      if (stay_compteur_ms >= stay_target_ms) {
        statut = DOWN;
        stay_compteur_ms = 0;
      }
      break;

      case DOWN:
      compteur_ms--;
      if (compteur_ms <= 0) {
        statut=STAY_DOWN;
      }
      break;

      case STAY_DOWN:
      stay_compteur_ms++;
      if (stay_compteur_ms >= stay_target_ms) {
        statut = UP;
        stay_compteur_ms = 0;
      }
      break;
    } 
      
    frequence = round(compteur_ms*((float)freq_haut-freq_bas)/periode_ms) + freq_bas;
    if (freq_prec != frequence) {
      freq_prec = frequence;
      setPWM(frequence, dutyCycle);
    }
  }
}

void setPWM (long frequence, float dutyCycle) {
  unsigned int dc = round(dutyCycle*255/100);
//  noInterrupts();
  SetPinFrequencySafe(LED_PIN1, frequence); // ajuster la frequence du PWM
  pwmWrite(LED_PIN1, dc); // ajuster le duty cycle du PWM

  SetPinFrequencySafe(LED_PIN2, frequence); // ajuster la frequence du PWM
  pwmWrite(LED_PIN2, dc); // ajuster le duty cycle du PWM
//  interrupts();
}

void setParam(int p, long fb, long fh, float dc) {
  periode = p;
  periode_ms = periode * 1000;
  compteur_ms = 0;
  stay_target_ms = 100;
  stay_compteur_ms = 0;
  statut = UP;
  
  freq_bas = fb;
  freq_haut = fh;
  frequence = freq_bas;
  freq_prec = freq_bas;
  
  dutyCycle = dc;

  setPWM(frequence, dutyCycle);
}
