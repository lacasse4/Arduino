#include <test.h>

void setup() {
  Serial.begin(9600);
  Serial.println("Ready");

}

void loop() {
  int a = 2; 
  int b = 2;
  int c = plus(a, b);
  Serial.println(c);
  while(1);
}
