/*
 * 
 * Programme LuxSonarium V2
 * ------------------------
 * Auteur: Vincent Lacasse
 * Date: 2019-08-10
 * 
 * Ce programme controle les pins 10 et 3 (canal 1 et 2 respectivement) d'un Arduino Uno V3 
 * a partir de commandes fournies sur le port serie. 
 * Ce programme a ete concu pour le projet Lux Sonarium de Martin Leduc.
 * Les deux pins sont destinees a commander deux strips de leds via des transistors de puissance.
 * Les pins donnent un signal PWM a frequence ajustable ce qui produit un effet stroboscopique. 
 * Le debut et la fin d'une sequence PWM peuvent etre ajustees independamment en forme de rampe 
 * en modifiant le duty cycle du PWM afin de permettre d'allumer et d'eteindre les strips doucement. 
 * 
 * Version 2 (2022-01-11):
 * - la commande DUTY a été ajoutée afin de pouvoir ajuster l'intensité
 * - les commandes peuvent être entrées en minuscules
 */

#include <PWM.h>  // PWM Frequency library available at https://code.google.com/archive/p/arduino-pwm-frequency-library/downloads
#include <Cmd.h>  // CMD library pour le traitement des commandes (Copyright (C) 2009 FreakLabs)

#define PIN_CANAL_1  10         // pin du header Arduino pour activer la strip 1
#define PIN_CANAL_2  3          // pin du header Arduino pour activer la strip 2
#define MIN_FREQ     32         // frequence minimum du PWM (Hz)
#define MAX_FREQ     5000       // frequence maximum du PWM (hz)
#define MIN_TEMPS    0          // temps mininim pour la rampe (ms)
#define MAX_TEMPS    60000      // temps maximum pour la rampe (ms)
#define MIN_DUTY     0          // duty cycle pour le OFF
#define MAX_DUTY     50         // duty cycle max pour le ON

#define ERR_NARG          1
#define ERR_CANAL         2
#define ERR_FREQ          3
#define ERR_TEMPS         4
#define ERR_LED_OFF       5
#define ERR_DUTY          6

#define INIT_RAMPE_MONTEE     2000   // rampe de montee initiale (ms) 
#define INIT_RAMPE_DESCENTE   2000   // rampe de descente initiale (ms)
#define INIT_FREQUENCE        150    // frequence initiale (Hz)
#define INIT_DUTY             ((float)0.25)  // duty cycle initial (0.0 - 1.0)

typedef enum { OFF, MONTE, ON, DESCEND } t_statut;

int pins[2] = { PIN_CANAL_1, PIN_CANAL_2 };

int debutCible[2];    // Periode de la rampe du début (ms)
int finCible[2];      // Periode de la rampe de fin (ms)
int debutCompteur[2]; // Compteur en ms pour la rampe de début
int finCompteur[2];   // Compteur en ms pour la rampe de fin
t_statut statut[2];   // statut de la pin
double dutyOn[2];     // duty cycle (controle d'intensité) 0.0 à 1.0

bool succes;          // true si le PMW est en fonction


void setup() {
  
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  Serial.flush();
  help();

  pinMode(LED_BUILTIN, OUTPUT); // Heart Beat LED

  InitTimersSafe();   // initialiser tous les timers sauf le 0 qui est utilise pour le millis()

  init1();
  
  cmdInit(&Serial);
  cmdAdd("ON", onCmd);
  cmdAdd("OFF",offCmd);
  cmdAdd("FREQ", setFreqCmd);
  cmdAdd("DEBUT", setDebutCmd);
  cmdAdd("FIN", setFinCmd);
  cmdAdd("INIT", initCmd);
  cmdAdd("DUTY", setDutyCmd);
  cmdAdd("HELP", helpCmd);
}

void help() {
  Serial.println("Lux Sonarium V2");
  Serial.println("---------------");
  Serial.println("Usage:");
  Serial.println();
  Serial.println("Entrer une commande parmis les suivantes:");
  Serial.println(" ON <canal>            : allume le canal");
  Serial.println(" OFF <canal>           : eteint le canal");
  Serial.println(" FREQ <canal> <freq>   : ajuste la frequence du canal specifie");
  Serial.println(" DEBUT <canal> <temps> : ajuste la rampe de debut du canal specifie");
  Serial.println(" FIN <canal> <temps>   : ajuste la rampe de fin du canal specifie");
  Serial.println(" INIT                  : retour à la configuration de demarrage");
  Serial.println(" DUTY <canal> <duty>   : ajuste l'intensite maximum du canal"); 
  Serial.println(" HELP                  : imprime ce message");
  Serial.println();
  Serial.println("Parametres:");
  Serial.println(" <canal>                : canal = 1 ou 2");
  Serial.println(" <freq>                 : frequence en Hertz, valeur entiere entre 32 et 5000");
  Serial.println(" <temps>                : temps en millisecondes, valeur entiere entre 0 et 60000 (60 sec.)");
  Serial.println(" <duty>                 : duty cycle, valeur entiere entre 0 et 100");
  Serial.println();
  Serial.println("Au demarrage tous les canneaux sont ON avec freq = 150, duty = 25 et temps = 2000");
  Serial.println();  
}

void init1() 
{
  statut[0] = MONTE;
  statut[1] = MONTE;
  
  setDebut(0, INIT_RAMPE_MONTEE);  
  setDebut(1, INIT_RAMPE_MONTEE);  
  setFin(0, INIT_RAMPE_DESCENTE);  
  setFin(1, INIT_RAMPE_DESCENTE);  
  
  setFreq(0, INIT_FREQUENCE);     
  setFreq(1, INIT_FREQUENCE);

  setDuty(0, INIT_DUTY);
  setDuty(1, INIT_DUTY);
}


void loop() {
  cmdPoll();

  unsigned long thisMillis = millis();
  lookAlive(thisMillis);
  traiterCanaux(thisMillis);  
}

void traiterCanaux(unsigned long thisMillis) {
  static unsigned long lastMillis = 0;
  static unsigned long interval = 1;
  
  if (thisMillis - lastMillis >= interval) {
    lastMillis = thisMillis;
    traiterCanal(0);
    traiterCanal(1);
  }
}

void lookAlive(unsigned long thisMillis) 
{
  static boolean alive_on = false;
  static unsigned long lastMillis = 0;
  static unsigned long interval = 500;
  
  if (thisMillis - lastMillis >= interval) {
    lastMillis = thisMillis;
    alive_on = !alive_on;
    digitalWrite(LED_BUILTIN, alive_on ? HIGH : LOW);
  }
}

void traiterCanal(int canal) 
{
  int dutyCycle;

  // traiter la montée de l'intensité
  if (statut[canal] == MONTE) {
    debutCompteur[canal]++;
    if (debutCompteur[canal] >= debutCible[canal]) {
      setStatut(canal, ON);
    }
    dutyCycle = getDutyCycle(canal);
    pwmWrite(pins[canal], dutyCycle);
  }  

  // traiter la baisse d'intensité
  if (statut[canal] == DESCEND) {
    finCompteur[canal]++;
    if (finCompteur[canal] >= finCible[canal]) {
      setStatut(canal, OFF);
    }
    dutyCycle = getDutyCycle(canal);
    pwmWrite(pins[canal], dutyCycle);
  }
}  

void setDebut(int canal, int periode) 
{
  debutCible[canal] = periode;
  debutCompteur[canal] = 0;
}

void setFin(int canal, int periode) 
{
  finCible[canal] = periode;
  finCompteur[canal] = 0;
}

void setFreq(int canal, long freq)
{
  int dutyCycle;
  
  // Ajuster la frequence du PWM
  succes = SetPinFrequencySafe(pins[canal], freq);  

  // Ajuster le duty cycle du PWM 
  // (doit toujours être fait après un ajustement de fréquence) 
  dutyCycle = getDutyCycle(canal);
  pwmWrite(pins[canal], dutyCycle);
}

void setDuty(int canal, double duty) {
  int dutyCycle;
  dutyOn[canal] = duty;
  dutyCycle = getDutyCycle(canal);
  pwmWrite(pins[canal], dutyCycle);  
}

int getDutyCycle(int canal) {
  int dutyCycle;
  
  switch (statut[canal]) {
  case OFF:
    dutyCycle = 0;
    break;
    
  case ON:
    dutyCycle = round(dutyOn[canal]*255);
    break;
    
  case MONTE:
    dutyCycle = round(dutyOn[canal]*debutCompteur[canal]*255.0/debutCible[canal]);
    break;
    
  case DESCEND:
    dutyCycle = round(dutyOn[canal]*(finCible[canal]-finCompteur[canal])*255.0/finCible[canal]);
    break;
  }
  return dutyCycle;
}

void setStatut(int canal, t_statut new_statut) {
  statut[canal] = new_statut;
  debutCompteur[canal] = 0;
  finCompteur[canal] = 0;
}

void initCmd(int argc, char **args) 
{
  init1();
}

void onCmd(int argc, char **args)
{
  int canal;

  if (argc != 2) {
    imprimeErreur(ERR_NARG);
    return;
  }
  
  canal = cmdStr2Num(args[1], 10) - 1;
  if (canal != 0 && canal != 1) {
    imprimeErreur(ERR_CANAL);
    return;
  }

  if (statut[canal] == ON || statut[canal] == MONTE) return;
  setStatut(canal, MONTE);
}

void offCmd(int argc, char **args)
{
  int canal;

  if (argc != 2) {
    imprimeErreur(ERR_NARG);
    return;
  }
  
  canal = cmdStr2Num(args[1], 10) - 1;
  if (canal != 0 && canal != 1) {
    imprimeErreur(ERR_CANAL);
    return;
  }

  if (statut[canal] == OFF || statut[canal] == DESCEND) return;
  setStatut(canal, DESCEND);
}

void setFreqCmd(int argc, char **args)
{
  int canal, freq;

  if (argc != 3) {
    imprimeErreur(ERR_NARG);
    return;
  }
  
  canal = cmdStr2Num(args[1], 10) - 1;
  if (canal != 0 && canal != 1) {
    imprimeErreur(ERR_CANAL);
    return;
  }

  freq = cmdStr2Num(args[2], 10);
  if (freq < MIN_FREQ || freq > MAX_FREQ) {
    imprimeErreur(ERR_FREQ);
    return;
  }

  setFreq(canal, freq);
}

void setDebutCmd(int argc, char **args)
{
  int canal, temps;

  if (argc != 3) {
    imprimeErreur(ERR_NARG);
    return;
  }
  
  canal = cmdStr2Num(args[1], 10) - 1;
  if (canal != 0 && canal != 1) {
    imprimeErreur(ERR_CANAL);
    return;
  }

  if (statut[canal] != OFF) {
    imprimeErreur(ERR_LED_OFF);
    return;
  }
    
  temps = cmdStr2Num(args[2], 10);
  if (temps < MIN_TEMPS || temps > MAX_TEMPS) {
    imprimeErreur(ERR_TEMPS);
    return;
  }

  debutCible[canal] = temps;
}

void setFinCmd(int argc, char **args)
{
  int canal, temps;

  if (argc != 3) {
    imprimeErreur(ERR_NARG);
    return;
  }
  
  canal = cmdStr2Num(args[1], 10) - 1;
  if (canal != 0 && canal != 1) {
    imprimeErreur(ERR_CANAL);
    return;
  }

  if (statut[canal] != OFF) {
    imprimeErreur(ERR_LED_OFF);
    return;
  }
    
  temps = cmdStr2Num(args[2], 10);
  if (temps < MIN_TEMPS || temps > MAX_TEMPS) {
    imprimeErreur(ERR_TEMPS);
    return;
  }

  finCible[canal] = temps;
}

void setDutyCmd(int argc, char **args)
{
  int canal, duty;

  if (argc != 3) {
    imprimeErreur(ERR_NARG);
    return;
  }
  
  canal = cmdStr2Num(args[1], 10) - 1;
  if (canal != 0 && canal != 1) {
    imprimeErreur(ERR_CANAL);
    return;
  }
    
  duty = cmdStr2Num(args[2], 10);
  if (duty < MIN_DUTY || duty > MAX_DUTY) {
    imprimeErreur(ERR_DUTY);
    return;
  }

  setDuty(canal, (double)duty/100);
}

void helpCmd(int argc, char **args) 
{
  help();
}

void imprimeErreur(int code) {
  switch (code) {
    case ERR_NARG:
      Serial.print("*** Erreur: mauvais nombre d'agruments");
      break;

    case ERR_CANAL:
      Serial.print("*** Erreur: canal invalide, utiliser 1 ou 2");
      break;

    case ERR_FREQ:
      Serial.print("*** Erreur: frequence invalide");
      break;
    
    case ERR_TEMPS:
      Serial.print("*** Erreur: temps invalide ");
      break;
    
    case ERR_LED_OFF:
      Serial.print("*** Erreur: le canal doit etre OFF pour modifier le temps");
      break;
    
    case ERR_DUTY:
      Serial.print("*** Erreur: duty invalide");
      break;
    
    default:
      Serial.print("*** Erreur inconnue");
      break;
  }
  Serial.println(" ***");
}
