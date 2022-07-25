#include <DueTimer.h>

int alive;


void handler(void) {
  digitalWrite(LED_BUILTIN, alive);
  alive = !alive;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  alive = 0;

  Timer3.attachInterrupt(handler).start(100000); // flash every 0.2 sec
}

void loop() {

}
