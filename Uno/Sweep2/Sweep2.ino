#include <PWM.h>                  // PWM Frequency library available at https://code.google.com/archive/p/arduino-pwm-frequency-library/downloads

// VL: Mi bas de la guitare (E2): 82.41 Hz

#define LED_PIN1 10          // pin Arduino pour activer les LED
#define LED_PIN2 3
#define MAX_DUTY_CYCLE 50.0 // duty cycle maximum pour proteger le circuit (%)
#define MIN_FREQ 32         // frequence minimum du PWM (Hz)
#define MAX_FREQ 62500      // frequence maximum du PWM (hz)
#define MIN_PERIODE 1       // periode minimum (sec)
#define MAX_PERIODE 600     // periode maximum (sec)

#define ERR_MAX_DUTY      1
#define ERR_SETPINFREQ    2
#define ERR_LECTURE       3
#define ERR_MIN_PERIODE   4
#define ERR_MAX_PERIODE   5
#define ERR_MIN_FREQ      6
#define ERR_MAX_FREQ      7
#define ERR_FREQ_ORDRE    8 

long freq_bas;        // Frequence basse de pulsation des LED (Hz)
long freq_haut;       // Frequence haute de pulsation de LED (Hz)
long frequence;       // Frequence de pulsation courante (Hz)
long freq_prec;       // Frequence de pulsation precedente
float dutyCycle;      // Duty cycle du PWM de 1 à 100
int periode;          // Periode du sweep (sec)
int periode_ms;       // Periode du sweep en ms.
int compteur_ms;      // Compteur de ms pour parcourir la periode
bool monte;           // true si on est sur la montee de la periode

unsigned long this_millis;
unsigned long last_millis = 0;
bool succes;          // true si le PMW est en fonction

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  Serial.flush();

  pinMode(LED_BUILTIN, OUTPUT); // Heart Beat LED

  InitTimersSafe();   // initialiser tous les timers sauf 0 qui est utilise pour le time()
  
  Serial.println("Sweep V1");
  Serial.println("---------------------");
  Serial.println(">>> Entrez periode (s), frequence basse (Hz), frequence haute (Hz) et duty cycle (%)");
  Serial.println();

  setStatut(
    2,       // periode (sec)
    100,       // frequence basse (Hz)
    1000,       // frequence haute (Hz)
    50);      // duty cycle (%)
}

void loop() {

  if (monte) digitalWrite(LED_BUILTIN, HIGH);
  else       digitalWrite(LED_BUILTIN, LOW);

  lectureParam();

  this_millis = millis();
  if (last_millis != this_millis) {
    last_millis = this_millis;
    if (monte) {
      compteur_ms++;
      if (compteur_ms >= periode_ms) {
        monte = false;
      }
    }
    else {
      compteur_ms--;
      if (compteur_ms <= 0) {
        monte = true;
      }
    }
    frequence = round(compteur_ms*((float)freq_haut-freq_bas)/periode_ms) + freq_bas;
    if (freq_prec != frequence) {
      freq_prec = frequence;
      succes = SetPinFrequencySafe(LED_PIN1, frequence); // ajuster la frequence du PWM
      succes = SetPinFrequencySafe(LED_PIN2, frequence); // ajuster la frequence du PWM
      pwmWrite(LED_PIN1, round(dutyCycle*255/100)); // ajuster le duty cycle du PWM
      pwmWrite(LED_PIN2, round(dutyCycle*255/100)); // ajuster le duty cycle du PWM
      Serial.println(frequence);
    }
  }
}

void imprimeStatut() {
  Serial.print("Periode: ");
  Serial.println(periode);
  Serial.print("Frequence basse: ");
  Serial.println(freq_bas);
  Serial.print("Frequence haute: ");
  Serial.println(freq_haut);
  Serial.print("Duty cycle: ");
  Serial.println(dutyCycle);
  Serial.println();
}

void imprimeErreur(int code) {
  switch (code) {
    case ERR_MAX_DUTY:
      Serial.print("*** Erreur: duty cycle maximum ");
      Serial.print(MAX_DUTY_CYCLE);
      break;

    case ERR_SETPINFREQ:
      Serial.print("*** Erreur: SetPinFrequencySafe");
      break;

    case ERR_LECTURE:
      Serial.print("*** Erreur de lecture");
      break;

    case ERR_MIN_PERIODE:
      Serial.print("*** Erreur: la periode minimum ");
      Serial.print(MIN_PERIODE);
      break;
    
    case ERR_MAX_PERIODE:
      Serial.print("*** Erreur: la periode maximum ");
      Serial.print(MAX_PERIODE);
      break;
    
    case ERR_MIN_FREQ:
      Serial.print("*** Erreur: la frequence minimum ");
      Serial.print(MIN_FREQ);
      break;
    
    case ERR_MAX_FREQ:
      Serial.print("*** Erreur: la frequence maximum ");
      Serial.print(MAX_FREQ);
      break;
    
    case ERR_FREQ_ORDRE:
      Serial.print("*** Erreur: la frequence minimum doit etre plus petite que la frequence maximum");
      break;
    
    default:
      Serial.print("*** Erreur inconnue");
      break;
  }
  Serial.println(" ***");
}

void lectureParam() {
  
  // des caracteres sont disponibles sur le port serie
  if (Serial.available() > 0) {
    
    // lire la période, la frequence basse, la frequence haute et le duty cycle
    int periode_tmp = Serial.parseInt();
    long freq_bas_tmp = Serial.parseInt();
    long freq_haut_tmp = Serial.parseInt();
    float duty_tmp = Serial.parseFloat();

    // verifier que la lecture du port serie s'est bien passee
    if (Serial.read() == '\n' && periode_tmp > 0 && freq_bas_tmp > 0 && freq_haut_tmp > 0 && duty_tmp > 0) {
      if (validerParam(periode_tmp, freq_bas_tmp, freq_haut_tmp, duty_tmp)) {
        setStatut(periode_tmp, freq_bas_tmp, freq_haut_tmp, duty_tmp);
      }
    }
    else {
      imprimeErreur(ERR_LECTURE);
    }
  }
}

boolean validerParam(int p, long fb, long fh, float dc) {
  boolean OK = false;

  if (p < MIN_PERIODE) imprimeErreur(ERR_MIN_PERIODE);
  else if (p > MAX_PERIODE) imprimeErreur(ERR_MAX_PERIODE);
  else if (fb < MIN_FREQ) imprimeErreur(ERR_MIN_FREQ);
  else if (fh > MAX_FREQ) imprimeErreur(ERR_MAX_FREQ);
  else if (fh < fb) imprimeErreur(ERR_FREQ_ORDRE);
  else if (dc > MAX_DUTY_CYCLE) imprimeErreur(ERR_MAX_DUTY);
  else OK = true;
  
  return OK;
}

// doit etre appele apres InitTimersSafe()
void setStatut(int p, long fb, long fh, float dc) {
  periode = p;
  periode_ms = periode * 1000;
  compteur_ms = 0;
  monte = true;
  
  freq_bas = fb;
  freq_haut = fh;
  frequence = freq_bas;
  freq_prec = freq_bas;
  
  dutyCycle = dc;

  succes = SetPinFrequencySafe(LED_PIN1, frequence); // ajuster la frequence du PWM
  succes = SetPinFrequencySafe(LED_PIN2, frequence); // ajuster la frequence du PWM
  succes = true;
  if(succes) {
    digitalWrite(LED_BUILTIN, HIGH);    
    pwmWrite(LED_PIN1, round(dutyCycle*255/100)); // ajuster le duty cycle du PWM
    pwmWrite(LED_PIN2, round(dutyCycle*255/100)); // ajuster le duty cycle du PWM
    imprimeStatut();
  }
  else {
    imprimeErreur(ERR_SETPINFREQ);
    digitalWrite(LED_BUILTIN, LOW);
  }
}
