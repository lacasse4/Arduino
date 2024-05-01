#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

#include "glcdfont.h"

Adafruit_MMA8451 mma = Adafruit_MMA8451();

#undef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

#define ON_TIME_US  400
#define OFF_TIME_US 600
#define LED_INIT_TIME_MS 500
#define LEAD_TIME_MS 100
#define DEAD_TIME_MS 100
#define ACC_THESHOLD 10.0

#define N_PINS 7
int pins[N_PINS] = {16, 17, 18, 19, 20, 21, 22};

const char *message = " VINCENT LACASSE ";
const char *empty   = "                 ";

void led_init(int status)
{
  for (int i = 0; i < N_PINS; i++) {
    pinMode(pins[i], OUTPUT);
  }

  unsigned int line = 1;
  for (int i = 0; i < N_PINS; i++) {
    set_leds(line);
    delay(LED_INIT_TIME_MS);
    if (status) line <<= 1;  // in case of error, flash the first led only
  }
  set_leds(0);
}

void set_leds(unsigned char c) 
{
  for (int i = 0; i < N_PINS; i++) {
    digitalWrite(pins[i], c & 1);
    c >>= 1;
  }
}

void drawChar(unsigned char c) {

    if (c >= 176) c++; // Handle 'classic' charset behavior

    for (int i = 0; i < 5; i++) { // Char bitmap = 5 columns
      unsigned char line = pgm_read_byte(&font[c * 5 + i]);
      set_leds(line);
      delayMicroseconds(ON_TIME_US);
      set_leds(0);
      delayMicroseconds(OFF_TIME_US);
    }
    delayMicroseconds(OFF_TIME_US);
}

void drawString(const char* s)
{
  if (s) {
    while (*s) {
      drawChar(*s++);
    }
  } 
}

void setup() {
  int status_ok = mma.begin();
  led_init(status_ok);
  if (status_ok) mma.setRange(MMA8451_RANGE_2_G);
}

void loop() { 
  sensors_event_t event;
  mma.getEvent(&event);
  if (abs(event.acceleration.y) > ACC_THESHOLD) {
    delay(LEAD_TIME_MS);
    drawString(message);
    delay(DEAD_TIME_MS);
  }
}
