
/*
 * AutoStrobe 1
 * Author: Vincent Lacasse
 * Date: 2022-01-09
 * 
 * Runs on Arduino Due
 * 
 * Spinoff of FFTtest7 that finds an audio signal fundamental frequency (robust)
 * Additions:
 *   - drives 2 PWM channels with a frequency slighty lower than the audio signal
 *      * channel 0 is on Due pin 35
 *      * channel 1 is on Due pin 36
 */

#include <arduinoFFT.h>
#include <spectrum.h>
#include <peak.h>
#include <peak_list.h>
#include <pwm_lib.h>

/*
 * Signal processing defines
 */
#define CHANNEL          A0     // digitizer channel
#define LOW_PEAK_POWER   1000   // lowest peak power considered in peak search
#define MAX_FREQUENCY    900    // maximum frequency detected (Hz)
#define MIN_FREQUENCY    40     // minimum frequency detected (Hz)

#define PWM_MULTIPLIER   100000000   // hundredths of microseconds (1e-8 seconds)
#define INIT_FREQUENCY   200
#define INIT_PERIOD     (PWM_MULTIPLIER/INIT_FREQUENCY)
#define DUTY_FACTOR      0.25
#define INIT_DUTY       (INIT_PERIOD*DUTY_FACTOR)

/* 
 *  Frequency offest between fundamental frequency found in sound input and  
 *  pwm frequency output to LED. This induces a 'slow movement' perception in
 *  the vibrating object that produces the sound (eg. guitar string) due to
 *  he stroboscobic effect of the light
 */
#define FREQUENCY_OFFSET  1.0 

#define CONTINUITY_CYCLES 5 
#define TAPEROFF_CYCLES   4

/*
 *  Three leds to form a 'vue meter' to show signal strength
 *  One led to indicate if there is input signal clipping
 */
#define PIN_POWER_1  2
#define PIN_POWER_2  3
#define PIN_POWER_3  4
#define PIN_CLIP     5

#define LOOP

/* 
 * In order to eliminate signal alaising during the digitization process,
 * the signal sampling frequency must be at least twice as much as the maximum
 * frequency present in the analog signal (as per Nyqvist rate).
 * Here, we use a 4000 Hz sampling frequency. This is 4 times greater that 
 * the analog signal frequency which is limited by an analog low pass filter
 * with a cut off frequency of 870 Hz
 */
const double samplingFrequency = 4000.0; // in Hz, must be less than 10000 Hz

/*
 * With a sampling frequency of 4000 Hz and a sample number of 1024, the minimum
 * frequency that can be detected is approximatly 8 Hz. This is low enough 
 * to detect the minimum audible frequency which is approx. 20 Hz
 */
const int signalLength = 1024;

/*
 * Signal object
 */
signal_t* sig;

/*
 * PWM
 */
using namespace arduino_due::pwm_lib;
pwm<pwm_pin::PWMH0_PC3> pwm_red;   //pwm_pin35;
pwm<pwm_pin::PWMH1_PC5> pwm_green; //pwm_pin37;
pwm<pwm_pin::PWMH2_PC7> pwm_blue;  //pwm_pin35;


/* for debugging
char ttt[3][100];
char uuu[3][100];
*/

/*
 * Other global variables
 */
int status;
int counter;
uint32_t period;
uint32_t duty;
uint32_t trans_duty;

void setup()
{
  Serial.begin(9600);
  Serial.println("AutoStrobe1");

  counter    = 0;
  period     = round(INIT_PERIOD);
  duty       = round(INIT_DUTY); 
  trans_duty = round(INIT_DUTY);

  sig = create_signal(signalLength, samplingFrequency, ZERO_PADDING_ENABLED);  
  pwm_red.start(period, duty);
  pwm_green.start(period, duty);
  pwm_blue.start(period, duty);
}

void loop()
{
  int size;

  // acquire signal
  acquire(sig, CHANNEL);


  // compute spectrum using a FFT
  compute_spectrum(sig);

  // compute the peak list
  compute_peak_list(sig, MIN_FREQUENCY, MAX_FREQUENCY);
  peak_list_t* pl = get_peak_list(sig);
  peak_t fundamental = find_fundamental_frequency(pl);

  size = list_size(pl);
  if (size == 0) {        // no fundamental found, lights off gradually
    if (counter >= CONTINUITY_CYCLES + TAPEROFF_CYCLES) {
      trans_duty = 0;
      pwm_pin35.stop();
    }
    else {
      counter++;
      if (counter >= CONTINUITY_CYCLES) {
        trans_duty -= duty / 4;
        trans_duty = trans_duty > duty ? 0 : trans_duty;  // make sure there no overflow.
      }
    }
    status = pwm_pin35.set_duty(trans_duty);
  } 
  else {
    counter = 0;
    period     = round(PWM_MULTIPLIER / (fundamental.frequency + 1));  
    duty       = round(period * DUTY_FACTOR);  
    trans_duty = duty;       

    pwm_pin35.stop();
    pwm_pin35.start(period, duty);
  }

  printFrequency(fundamental);
//  printSignal();
  
//  delay(10000);
//  Serial.println();
}

void printFrequency(peak_t peak) 
{
  char s1[30], s2[30];
  static int heart_beat = 0;
  if (heart_beat) {
    strcpy(s1, " *");
    heart_beat = 0;
  }
  else {
    strcpy(s1, "  ");
    heart_beat = 1;
  }
  
  sprintf(s2, " F = %6.2f Hz\r", peak.frequency);
  strcat(s1, s2);
  Serial.print(s1);
}

void printPeak(peak_t peak) 
{
  char s[200];
  sprintf(s, "Peak: %3d, %5.1f, %7.1f", peak.index, peak.frequency, peak.power);
  Serial.println(s);
  /*sprintf(s, "Debug: %1d, %1d, %5.1f, %5.1f, %5.1f, %5.1f, %5.1f, %5.1f", peak.ispeak, peak.added, peak.p[0], peak.p[1], peak.p[2], peak.p[3], peak.p[4], peak.early_freq);
  Serial.println(s);*/
}

void printd(double d) {
  char s[100];
  sprintf(s, "%4.2f", d);
  Serial.println(s);
}

void printSignal() {
  char s[100];
  double *a = get_signal_array(sig);
  int len = get_length(sig);
  for (int i = 0; i < len; i++) {
    sprintf(s, "%4d %5.1f", i, a[i]);
    Serial.println(s);
  }
}
