#include "Adafruit_HT1632.h"

#define HT_DATA 2
#define HT_WR   3
#define HT_CS   4
#define HT_CS2  5

// use this line for single matrix
Adafruit_HT1632LEDMatrix matrix = Adafruit_HT1632LEDMatrix(HT_DATA, HT_WR, HT_CS);
// use this line for two matrices!
//Adafruit_HT1632LEDMatrix matrix = Adafruit_HT1632LEDMatrix(HT_DATA, HT_WR, HT_CS, HT_CS2);
int i = 0;
char buff[100];

void setup() {
  Serial.begin(9600);
  matrix.begin(ADA_HT1632_COMMON_16NMOS);
  matrix.clearScreen();
  matrix.setRotation(0);
  matrix.setTextWrap(false);
}

void loop() {

    sprintf(buff, "%4d", i);
    i++;
    if (i == 10000) i = 0;
    

    matrix.setTextColor(0x1, 0);
    matrix.setCursor(0, 0);
    matrix.print(buff);
    matrix.setCursor(0, 9);
    matrix.print(buff);   
    matrix.writeScreen();    
    
    delay(50);
    
    matrix.setTextColor(0x0, 0);
    matrix.setCursor(0, 0);
    matrix.print(buff);
    matrix.setCursor(0, 9);
    matrix.print(buff);   
    matrix.writeScreen();    

}
