#include "pwm_lib.h"

// VL: Mi bas de la guitare (E2): 82.41 Hz

using namespace arduino_due::pwm_lib;

/* 
 * With the pwm_lib library, PERIOD and DUTY 
 * must be specified in hundredths of microseconds (1e-8 seconds)
 */
#define PERIOD (100000000/82)
#define DUTY   (PERIOD/2)

pwm<pwm_pin::PWMH0_PC3> pwm_pin35;

void setup() {
    pwm_pin35.start(PERIOD, DUTY);
}

void loop() {
  // put your main code here, to run repeatedly:

}
