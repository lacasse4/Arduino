
/*
 * AutoStrobe 2
 * Author: Vincent Lacasse
 * Date: 2022-07-25
 * 
 * Runs on Arduino Due
 * 
 * Finds an audio signal fundamental frequency
 * Updated from AutoStrobe 1 following construction of hardware prototype
 * Additions:
 *   - data acquisition is driven by a timer interrupt
 *   - drives 3 PWM channels with a frequency slighty lower than the audio signal
 *      * channel Red is on Due pin 35
 *      * channel Green is on Due pin 37
 *      * channel Blue is on Due pin 39
 *    - drives 3 on/off led as signal power vu-meter
 *    - drives 1 on/off led as clipping signal
 */

// #include <arduinoFFT.h>
#include <spectrum.h>
#include <peak.h>
#include <peak_list.h>
#include <pwm_lib.h>
#include <DueTimer.h>

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

#define ALPHA           0.01

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
 * Signal objects
 * two channels are required. One signal is acquired will the other one is processed
 */
int current_sample = 0;
int sample_index = 0;
int acquisition_channel = 0;
int processing_channel = 1;
signal_t* sig[2];

/*
 * PWM
 */
using namespace arduino_due::pwm_lib;
pwm<pwm_pin::PWMH0_PC3> pwm_red;   //pwm_pin35;
pwm<pwm_pin::PWMH1_PC5> pwm_green; //pwm_pin37;
pwm<pwm_pin::PWMH2_PC7> pwm_blue;  //pwm_pin39;

/*
 * Alpha filter
 */
double smooth = 0.0;

/* for debugging
char ttt[3][100];
char uuu[3][100];
*/

/*
 * Other global variables
 */
int alive;
int counter;
uint32_t period;
uint32_t duty;
uint32_t trans_duty;

void acquisition_handler(void) {

  // disable interrupts
  noInterrupts();
  
  current_sample = analogRead(CHANNEL);
  add_sample(sig[acquisition_channel], current_sample);

  // enable interrupts
  interrupts();
}



void setup()
{
  Serial.begin(9600);
  Serial.println("AutoStrobe2\n");

  pinMode(LED_BUILTIN, OUTPUT); digitalWrite(LED_BUILTIN, 0);
  pinMode(PIN_POWER_1, OUTPUT); digitalWrite(PIN_POWER_1, 0);
  pinMode(PIN_POWER_2, OUTPUT); digitalWrite(PIN_POWER_2, 0);
  pinMode(PIN_POWER_3, OUTPUT); digitalWrite(PIN_POWER_3, 0);
  pinMode(PIN_CLIP   , OUTPUT); digitalWrite(PIN_CLIP   , 0);

  alive      = 0;
  counter    = 0;
  period     = round(INIT_PERIOD);
  duty       = round(INIT_DUTY); 
  trans_duty = round(INIT_DUTY);

  // create 2 signal channels 
  sig[0] = create_signal(signalLength, samplingFrequency, ZERO_PADDING_ENABLED);  
  sig[1] = create_signal(signalLength, samplingFrequency, ZERO_PADDING_ENABLED);  

  // start RBG PWM
  led_start(period, duty);

  // start signal acquisition
  Timer3.attachInterrupt(acquisition_handler);
  Timer3.setFrequency(samplingFrequency); 
  Timer3.start();
}

void loop()
{
  int pl_size;
  int is_full;
  int sample;

  // get buffer status (within mutex)
  noInterrupts();
    is_full = is_buffer_full(sig[acquisition_channel]);
    sample = current_sample;
  interrupts();

  // alpha filter for vu meter
  smooth = smooth * (1.0 - ALPHA) + abs(sample-512) * ALPHA;
  display_vu_meter(smooth);

  // check if data acquisition has completed
  if (!is_full) {
    return;
  }

  /*
   * The acquistion buffer is full.  Process the signal.
   */

  // enter critical zone (mutex with isr)
  noInterrupts();

    // swap acquisition channel
    int temp = acquisition_channel;
    acquisition_channel = processing_channel;
    processing_channel = temp;

    // get ready for the next acquistion
    erase_signal(sig[acquisition_channel]);

  // exit critical zone
  interrupts();

  // flash LED_BUILTIN to show the program is running.
  digitalWrite(LED_BUILTIN, alive);
  alive = !alive;

  // compute spectrum using a FFT
  compute_spectrum(sig[processing_channel]);

  // compute the peak list
  compute_peak_list(sig[processing_channel], MIN_FREQUENCY, MAX_FREQUENCY);
  peak_list_t* pl = get_peak_list(sig[processing_channel]);
  peak_t fundamental = find_fundamental_frequency(pl);

  pl_size = list_size(pl);
  if (pl_size == 0) {        // no fundamental found, lights off gradually
    if (counter >= CONTINUITY_CYCLES + TAPEROFF_CYCLES) {
      trans_duty = 0;
      led_stop();
    }
    else {
      counter++;
      if (counter >= CONTINUITY_CYCLES) {
        trans_duty -= duty / 4;
        trans_duty = trans_duty > duty ? 0 : trans_duty;  // make sure there no overflow.
      }
    }
    led_set_duty(trans_duty);
  } 
  else {
    counter = 0;
    period     = round(PWM_MULTIPLIER / (fundamental.frequency + 1));  
    duty       = round(period * DUTY_FACTOR);  
    trans_duty = duty;       

    led_stop();
    led_start(period, duty);
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

void printSignal(int index) {
  char s[100];
  double *a = get_signal_array(sig[index]);
  int len = get_length(sig[index]);
  for (int i = 0; i < len; i++) {
    sprintf(s, "%4d %5.1f", i, a[i]);
    Serial.println(s);
  }
}

void led_start(uint32_t period, uint32_t duty) {
  pwm_red.start(period, duty);
  pwm_green.start(period, duty);
  pwm_blue.start(period, duty);  
}

void led_stop() {
  pwm_red.stop();
  pwm_green.stop();
  pwm_blue.stop();
}

void led_set_duty(uint32_t duty) {
  pwm_red.set_duty(duty);
  pwm_green.set_duty(duty);
  pwm_blue.set_duty(duty);
}

void display_vu_meter(double smooth) {
  if (smooth < 50) {
     digitalWrite(PIN_POWER_1, 0);
     digitalWrite(PIN_POWER_2, 0);
     digitalWrite(PIN_POWER_3, 0);
     digitalWrite(PIN_CLIP   , 0);
  }
  else if (smooth < 150) {
     digitalWrite(PIN_POWER_1, 1);
     digitalWrite(PIN_POWER_2, 0);
     digitalWrite(PIN_POWER_3, 0);
     digitalWrite(PIN_CLIP   , 0);
    
  }
  else if (smooth < 300) {
     digitalWrite(PIN_POWER_1, 1);
     digitalWrite(PIN_POWER_2, 1);
     digitalWrite(PIN_POWER_3, 0);
     digitalWrite(PIN_CLIP   , 0);
    
  }
  else if (smooth < 450) {
     digitalWrite(PIN_POWER_1, 1);
     digitalWrite(PIN_POWER_2, 1);
     digitalWrite(PIN_POWER_3, 1);
     digitalWrite(PIN_CLIP   , 0);    
  }
  else {
     digitalWrite(PIN_POWER_1, 1);
     digitalWrite(PIN_POWER_2, 1);
     digitalWrite(PIN_POWER_3, 1);
     digitalWrite(PIN_CLIP,    1);
  }
}
