#define PWM_PIN 11
#define DIR_PIN 10
#define FG_PIN  9

#define ERR_NARG          1
#define ERR_SPEED         2

#include <Cmd.h>  // CMD library pour le traitement des commandes (Copyright (C) 2009 FreakLabs)

void setup() { 
  Serial.begin(9600); 
  help();

  cmdInit(&Serial);
  cmdAdd("RUN",  runCmd);
  cmdAdd("STOP", stopCmd);
  cmdAdd("HELP", helpCmd);

  pinMode(DIR_PIN, OUTPUT); // set direction control PIN as output 
  pinMode(PWM_PIN, OUTPUT); // set PWM PIN as output 

  /* 
   *  DIR_PIN (yellow cable) down not seem to work for direction control.
   *  But it has to be set to 1 to work anyway.
   *  
   *  Couldn't make sense out of FG_PIN which is an output from the motor 
   */  
  digitalWrite(DIR_PIN, 1);

  // stop the motor
  digitalWrite(PWM_PIN, 255);

} 

void help() {
  Serial.flush();
  delay(1000);
  Serial.println();
  Serial.println("FIT0441-Max V1");
  Serial.println("--------------");
  Serial.println("Usage:");
  Serial.println();
  Serial.println("Entrer une commande parmis les suivantes:");
  Serial.println(" RUN <vitesse> : faire tourner le moteur à la vitesse <vitesse>");
  Serial.println("                 <vitesse> doit être une valeur comprise entre 0 (stop) et 255 (max)");
  Serial.println(" STOP          : arrêter le moteur (identique à RUN 0)");
  Serial.println(" HELP          : imprime ce message");
  Serial.println();
}

void loop() {
  cmdPoll();
}


void runCmd(int argc, char **args)
{
  int speed;
  int command;

  if (argc != 2) {
    printError(ERR_NARG);
    return;
  }

  speed = cmdStr2Num(args[1], 10);
  if (speed < 0 || speed > 255) {
    printError(ERR_SPEED);
    return;
  }

  command = 255 - speed;
  analogWrite(PWM_PIN, 255); // to avoid stalling at low speed
  analogWrite(PWM_PIN, command);
  Serial.println("OK");
}

void stopCmd(int argc, char **args)
{
  digitalWrite(PWM_PIN, 255);
  Serial.println("OK");
}

void helpCmd(int argc, char **args) 
{
  help();
}

void printError(int code) {
  switch (code) {
    case ERR_NARG:
      Serial.print("*** Erreur: mauvais nombre d'agruments");
      break;

    case ERR_SPEED:
      Serial.print("*** Erreur: vitesse invalide, la vitesse doit être comprise entre 0 et 255");
      break;
    
    default:
      Serial.print("*** Erreur inconnue");
      break;
  }
  Serial.println(" ***");
}
