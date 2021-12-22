// Capture un signal avec une fréquence d'échantillonage de 1 KHz et l'affiche à la console.

#define SIZE 512

int sensorValue;
int stringMiddle = 81;
int analogMiddle = 340;
int amplifier = 1;
int charIndex;
unsigned int oldChar;
unsigned int star = '*';
String s = "|                                                                               |                                                                               |";  
char n[10];
boolean stop = false;

unsigned int signal[SIZE];

unsigned int lastMillis = 0;
unsigned int thisMillis;
int compteur = 0;

void setup() {
  Serial.begin(250000);
  Serial.println("MicTest3");
}

void loop() {
  if (stop) return;
  
  thisMillis = millis();
  if (thisMillis > lastMillis) {
    lastMillis = thisMillis;
    signal[compteur++] = analogRead(A0);
    if (compteur >= SIZE) {
      printSignal(signal, SIZE);
      stop = true;
    }
  }
}

void printSignal(unsigned int signal[], int size) {
  for (int i = 0; i < size; i++) {
    charIndex = (signal[i] - analogMiddle) * amplifier + stringMiddle;
    oldChar = s.charAt(charIndex);
    s.setCharAt(charIndex, star);
//    sprintf(n, "%4d ", i);
//    Serial.print(n); 
    Serial.println(s);
    s.setCharAt(charIndex, oldChar);
  }
}
