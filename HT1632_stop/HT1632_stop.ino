#include "Adafruit_HT1632.h"

#define HT_DATA 2
#define HT_WR   3
#define HT_CS   4
#define HT_CS2  5

// use this line for single matrix
Adafruit_HT1632LEDMatrix matrix = Adafruit_HT1632LEDMatrix(HT_DATA, HT_WR, HT_CS);
// use this line for two matrices!
//Adafruit_HT1632LEDMatrix matrix = Adafruit_HT1632LEDMatrix(HT_DATA, HT_WR, HT_CS, HT_CS2);

void setup() {
  Serial.begin(9600);
  matrix.begin(ADA_HT1632_COMMON_16NMOS);
  matrix.clearScreen();
  matrix.setRotation(0);
  matrix.setTextWrap(false);
}


void loop() {
  // put your main code here, to run repeatedly:

}
