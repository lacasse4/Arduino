
#include <pwm_lib.h>


#define PWM_MULTIPLIER   100000000   // hundredths of microseconds (1e-8 seconds)
#define INIT_FREQUENCY   200
#define INIT_PERIOD     (PWM_MULTIPLIER/INIT_FREQUENCY)
#define DUTY_FACTOR      0.25
#define INIT_DUTY       (INIT_PERIOD*DUTY_FACTOR)


using namespace arduino_due::pwm_lib;
pwm<pwm_pin::PWMH0_PC3> pwm_pin35;
pwm<pwm_pin::PWMH1_PC5> pwm_pin37;
pwm<pwm_pin::PWMH2_PC7> pwm_pin39;

int status;
int counter;
uint32_t period;
uint32_t duty;
double d[] = {1.00, 0.95, 0.80, 0.70, 0.60, 0.40 };
#define N 6


void setup() {
  counter    = 0;
  period     = round(INIT_PERIOD);
  duty       = round(INIT_DUTY); 

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(13, OUTPUT);

  pwm_pin35.start(period, duty);
  pwm_pin37.start(period, duty);
  pwm_pin39.start(period, duty);

}

void loop() {
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(13, LOW);
  delay(500);
  
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(13, HIGH);
  delay(500);

  duty = round(INIT_PERIOD * d[counter]);
  counter++;
  if (counter >= N) counter = 0;

  pwm_pin35.stop();
  pwm_pin37.stop();
  pwm_pin39.stop();

  pwm_pin35.start(period, duty);
  pwm_pin37.start(period, duty);
  pwm_pin39.start(period, duty);
}
