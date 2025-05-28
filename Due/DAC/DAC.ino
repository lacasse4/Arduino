
#include "E2.h"

#define DELAY_US      100   // microseconds
#define SAMPLING_FREQ 10000 // Hz
#define NUM_SAMPLES   100000

int i = 0;
int string = 0;
uint16_t *data;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  analogWriteResolution(12);  // set the analog output resolution to 12 bit (4096 levels)
}

void loop() {

  data = data_e2;
  for (int i = 0; i < 6; i++) {
    flash_string(i+1);

    for (int j = 0; j < NUM_SAMPLES; i++) {
      analogWrite(DAC0, data[j]);
    }
  }

  delayMicroseconds(DELAY_US);
}


void flash_string(int string) {
  for (int i = 0; i < string; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}
