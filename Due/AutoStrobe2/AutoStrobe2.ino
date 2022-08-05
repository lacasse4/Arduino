
/*
 * AutoStrobe 2
 * Author: Vincent Lacasse
 * Date: 2022-07-25
 * 
 * Runs on Arduino Due
 * 
 * Finds audio signal fundamental frequency and produces pulse close to  this frequency
 * Updated from AutoStrobe 1 following construction of a hardware prototype
 * Additions:
 *   - data acquisition is driven by a timer interrupt
 *   - drives 3 PWM channels with a frequency slighty higher than the audio signal
 *      * channel Red is on Due pin 35
 *      * channel Green is on Due pin 37
 *      * channel Blue is on Due pin 39
 *    - drives 3 on/off led as signal power vu-meter
 *    - drives 1 on/off led as clipping signal
 *    - ramp up and dowm of LED intensity
 *    - the 3 PWM are new driven by an ISR rourine to ensure ramp up and down responsivness.
 *    - added a 1s watchdog to reset the program.
 */

#include <spectrum.h>
#include <peak.h>
#include <peak_list.h>
#include <pwm_lib.h>      // Copyright (C) 2015,2016 Antonio C. Domínguez Brito (<adominguez@iusiani.ulpgc.es>). 
                          // División de Robótica y Oceanografía Computacional (<http://www.roc.siani.es>) and 
                          // Departamento de Informática y Sistemas (<http://www.dis.ulpgc.es>). 
                          // Universidad de Las Palmas de Gran  Canaria (ULPGC) (<http://www.ulpgc.es>).
                          // GNU General Public License (GPL) license.
                          
#include <DueTimer.h>     // https://github.com/ivanseidel/DueTimer, 
                          // MIT License

/*
 * Signal processing 
 */
#define CHANNEL          A0     // Analog to Digital Converter channel
#define LOW_PEAK_POWER   1000   // lowest peak power considered in peak search
#define MAX_FREQUENCY    900    // maximum frequency detected (Hz)
                                // note: the analog signal is processed by an analog 
                                // low pass filter prior ADC (cut off = 870 Hz)
#define MIN_FREQUENCY    40     // minimum frequency detected (Hz)

/*
 * PMW for LED RGB output
 */
#define PWM_MULTIPLIER    100000000    // hundredths of microseconds (1e-8 seconds)
#define MAX_DUTY          0.5          // Max duty cycle in % (max LED intensity)
#define RAMP_UP_TIME      2.0          // seconds from no LED intensity to max intensity 
#define RAMP_DN_TIME      4.0          // seconds from max LED intensity to no intensity

#define UPDATE_INTERVAL   50           // interval between LED intensity updates in milliseconds
#define RAMP_UP_TIME_MS   (RAMP_UP_TIME*1000)
#define RAMP_DN_TIME_MS   (RAMP_DN_TIME*1000)

#define LED_OFF     0
#define RAMP_UP     1
#define RAMP_DN     2
#define LED_ON      3

/*
 * ALPHA filter is a 1st order IIR filter to smooth out the signal for vu meter
 * Smaller ALPHA is, more the signal is smooth (slowly varying)
 */
#define ALPHA           0.01

/* 
 *  Frequency offest between fundamental frequency found from sound input and  
 *  PWM frequency outputed to LED. This induces a 'slow movement' perception on
 *  the vibrating object that produced the sound (eg. guitar string) due to
 *  he stroboscobic effect of the light
 */
#define FREQUENCY_OFFSET  1.0 

/*
 *  Three green leds form a 'vu meter' to show audio signal strength
 *  One led indicates signal clipping
 */
#define VU_PIN_1    2
#define VU_PIN_2    3
#define VU_PIN_3    4
#define VU_CLIP     5

#define VU_OFF_THRS     30
#define VU_1LED_THRS   150
#define VU_2LED_THRS   300
#define VU_3LED_THRS   450

/* 
 * In order to eliminate signal alaising during the digitization process,
 * the analog signal is filtered by an analog low pass filter with a 870 Hz 
 * cut off frequency, using a Maxim MAX7404 hardwoar chip.
 * The signal sampling frequency must be at least twice as much as the maximum
 * frequency present in the analog signal (as per Nyqvist theorem).
 * We use a 4000 Hz sampling frequency. This is mare than 4 times greater that 
 * the analog signal frequency.
 */
const double samplingFrequency = 4000.0; // in Hz, must be less than 10000 Hz

/*
 * With a sampling frequency of 4000 Hz and a sample number of 1024, the minimum
 * frequency that can be detected is approximatly 8 Hz. This is low enough 
 * to detect the minimum audible frequency which is approx. 20 Hz
 */
const int signalLength = 1024;

/*
 * Signal processing variables
 * Two channels are required.
 * One channel is acquired via a timer interrupt handler 
 * while the other channel is processed in the main loop
 * 
 * Special care must be taken with variables that are shared between
 * the interrupt handler and the main loop.  Reads and writes from and to 
 * these variables must be placed in a 'critical zone' (by disabling 
 * interrupts)
 */

signal_t* sig[2];
volatile int acquisition_channel = 0;
volatile int processing_channel = 1;
volatile int npeaks = 0;
volatile double frequency;

/*
 * PWM objects to drive RGB LEDs
 */
using namespace arduino_due::pwm_lib;
pwm<pwm_pin::PWMH0_PC3> pwm_red;   //pwm_pin35;
pwm<pwm_pin::PWMH1_PC5> pwm_green; //pwm_pin37;
pwm<pwm_pin::PWMH2_PC7> pwm_blue;  //pwm_pin39;

/*
 * Alpha filter IIR tap
 */
volatile double smooth = 0.0;

/* for debugging
char ttt[3][100];
char uuu[3][100];
#define SKIP 2000000
*/
int  xxx = 0;


/*
 * Other global variables
 */
int alive = 0;              // To blink an 'Alive' signal on BUILTIN_LED

/*
 * Watchdog 
 */
#define WDT_KEY (0xA5)
#define WATCHDOG_DELAY  1   // watchdog delay in seconds

// watchdogSetup() is called from init()
// It must be redefined as empty function, otherwise watchdog will be disabled
void watchdogSetup(void) { } 

// enableWatchdog() enables the watchdow for a the period specified in 'seconds'
// Should be called in setup() 
void enableWatchdog(int seconds) {
  // WDT_MR_WDRSTEN -> triggers a processor reset 
  // (use WDT_MR_WDFIEN to call the interrupt handler 'void WDT_Handler(void)' instead
  // Slow clock is running at 32.768 kHz, watchdog frequency is therefore 32768 / 128 = 256 Hz
  // WDV holds the period in 256 th of seconds
  WDT->WDT_MR = WDT_MR_WDD(0xFFF) | WDT_MR_WDRSTEN | WDT_MR_WDV(256 * seconds); 
  NVIC_EnableIRQ(WDT_IRQn);
}

// restartWatchdog() should be called frequently in loop() to reset the watchdog timer.
void restartWatchdog(void) {
  WDT->WDT_CR = WDT_CR_KEY(WDT_KEY) | WDT_CR_WDRSTT;  
}

// WDT_Handler() not used in this program, for reference only
void WDT_Handler(void)  {
  WDT->WDT_SR; // Clear status register
}


/*
 * Data acquistion interrupt handler
 */
void acquisition_handler(void) {
  
  int sample;

  noInterrupts();
  
  sample = analogRead(CHANNEL);
  add_sample(sig[acquisition_channel], sample);
  smooth = smooth * (1.0 - ALPHA) + abs(sample-512) * ALPHA;

  interrupts();
}

/*
 * LED display interrupt handler
 */
void led_display_handler(void) {
  drivePWM(npeaks, frequency);
}

/*
 * Exectuted only once at startup.
 */
void setup()
{
  Serial.begin(9600);
  Serial.println("AutoStrobe2\n");

  pinMode(LED_BUILTIN, OUTPUT); digitalWrite(LED_BUILTIN, 0);
  pinMode(VU_PIN_1, OUTPUT);    digitalWrite(VU_PIN_1, 0);
  pinMode(VU_PIN_2, OUTPUT);    digitalWrite(VU_PIN_2, 0);
  pinMode(VU_PIN_3, OUTPUT);    digitalWrite(VU_PIN_3, 0);
  pinMode(VU_CLIP,  OUTPUT);    digitalWrite(VU_CLIP,  0);

  // create 2 signal channels 
  sig[0] = create_signal(signalLength, samplingFrequency, ZERO_PADDING_ENABLED);  
  sig[1] = create_signal(signalLength, samplingFrequency, ZERO_PADDING_ENABLED);  

  // stop RBG PWM
  led_stop();

  // start signal acquisition
  Timer0.attachInterrupt(acquisition_handler);
  Timer0.setFrequency(samplingFrequency); 
  // data_acquisition handler must have a higher priority than led display handler
  // otherwise data_acquisition rate is not stable.
  NVIC_SetPriority(TC0_IRQn, 0);         
  Timer0.start();

  Timer3.attachInterrupt(led_display_handler);
  Timer3.setPeriod(UPDATE_INTERVAL * 1000);
  NVIC_SetPriority(TC3_IRQn, 1);
  Timer3.start();

  enableWatchdog(WATCHDOG_DELAY);
}

/*
 * Executed continuously
 */
void loop()
{
  peak_list_t* peak_list;
  peak_t fundamental;
  
  volatile int is_full;
  volatile int smooth_safe;

  // get data acquisition status
  noInterrupts();   // enter critical zone
  
    is_full = is_buffer_full(sig[acquisition_channel]);
    smooth_safe = smooth;
    
  interrupts();     // exit critical zone

  // update vu meter display 
  display_vu_meter(smooth_safe);

  // ensure that data acquisition has completed
  if (!is_full) {
    return;
  }

  /*
   * The acquistion buffer is full.  Process the signal.
   */
  // First, prepare and launch data acquisition on the second channel 

  noInterrupts();    // enter critical zone

    // swap acquisition channel
    int temp = acquisition_channel;
    acquisition_channel = processing_channel;
    processing_channel = temp;

    // get ready for the next acquistion
    erase_signal(sig[acquisition_channel]);

  interrupts();     // exit critical zone

  // Then, process the signal on the first channel

  // compute spectrum using a FFT
  remove_bias(sig[processing_channel]);
  compute_spectrum(sig[processing_channel]);

  // compute the peak list
  compute_peak_list(sig[processing_channel], MIN_FREQUENCY, MAX_FREQUENCY);
  peak_list = get_peak_list(sig[processing_channel]);
  fundamental = find_fundamental_frequency(peak_list);
  
  noInterrupts();    // enter critical zone

    frequency = fundamental.frequency;
    npeaks = list_size(peak_list);

  interrupts();      // exit critical zone

  printFrequency(fundamental);

  // flash LED_BUILTIN to show the program is running.
  digitalWrite(LED_BUILTIN, alive);
  alive = !alive;

  restartWatchdog();
}


void drivePWM(int npeaks, double frequency) 
{
  static double last_valid_frequency;
  static int status = LED_OFF;
  static int ramp_up_counter;
  static int ramp_up_target;
  static int ramp_dn_counter;
  static int ramp_dn_target;

  uint32_t period;
  double precise_period;
  uint32_t duty;

  
  if (npeaks > 0) {
    last_valid_frequency = frequency;
  }

  // update PWM status, frequency and duty cycle
  switch (status) {

    case LED_OFF:
    led_stop();
    
    if (npeaks > 0) {
      status = RAMP_UP;
      ramp_up_counter = 0;
      ramp_up_target = RAMP_UP_TIME_MS;
    }
    break;
    
    case RAMP_UP:    
    precise_period = PWM_MULTIPLIER / (last_valid_frequency + FREQUENCY_OFFSET); 
    period = round(precise_period);
    duty   = round(precise_period * MAX_DUTY * ramp_up_counter / ramp_up_target);
    led_stop();
    led_start(period, duty);

    if (npeaks <= 0) {
      status = RAMP_DN;
      ramp_dn_counter = round(RAMP_DN_TIME_MS * (ramp_up_target-ramp_up_counter) / ramp_up_target);
      ramp_dn_target = RAMP_DN_TIME_MS;
    }
    else {
      ramp_up_counter += UPDATE_INTERVAL;
      if (ramp_up_counter >= ramp_up_target) {
        status = LED_ON;
      }
    }
    break;
    
    case RAMP_DN:
    precise_period = PWM_MULTIPLIER / (last_valid_frequency + FREQUENCY_OFFSET); 
    period = round(precise_period);
    duty   = round(precise_period * MAX_DUTY * (ramp_dn_target-ramp_dn_counter) / ramp_dn_target);
    led_stop();
    led_start(period, duty);

    if (npeaks > 0) {
      status = RAMP_UP;
      ramp_up_counter = round(RAMP_UP_TIME_MS * (ramp_dn_target-ramp_dn_counter) / ramp_dn_target);
      ramp_up_target = RAMP_UP_TIME_MS;
    }
    else { 
      ramp_dn_counter += UPDATE_INTERVAL;
      if (ramp_dn_counter >= ramp_dn_target) {
        status = LED_OFF;
      }
    }
    break;
    
    case LED_ON:
    precise_period = PWM_MULTIPLIER / (last_valid_frequency + FREQUENCY_OFFSET); 
    period = round(precise_period);
    duty   = round(precise_period * MAX_DUTY);
    led_stop();
    led_start(period, duty);
    
    if (npeaks <= 0) {
      status = RAMP_DN;
      ramp_dn_counter = 0;
      ramp_dn_target = RAMP_DN_TIME_MS;
    }
    break;
  }
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
  
  sprintf(s2, " F = %8.2f Hz\r", peak.frequency);
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
  if (smooth < VU_OFF_THRS) {           // All LEDs off
     digitalWrite(VU_PIN_1, 0);
     digitalWrite(VU_PIN_2, 0);
     digitalWrite(VU_PIN_3, 0);
     digitalWrite(VU_CLIP, 0);
  }
  else if (smooth < VU_1LED_THRS) {     // Green LED 1 on
     digitalWrite(VU_PIN_1, 1);
     digitalWrite(VU_PIN_2, 0);
     digitalWrite(VU_PIN_3, 0);
     digitalWrite(VU_CLIP, 0);
    
  }
  else if (smooth < VU_2LED_THRS) {     // Green LEDs 1 and 2 on
     digitalWrite(VU_PIN_1, 1);
     digitalWrite(VU_PIN_2, 1);
     digitalWrite(VU_PIN_3, 0);
     digitalWrite(VU_CLIP, 0);
    
  }
  else if (smooth < VU_3LED_THRS) {     // Green LEDs 1, 2 and 3 on
     digitalWrite(VU_PIN_1, 1);
     digitalWrite(VU_PIN_2, 1);
     digitalWrite(VU_PIN_3, 1);
     digitalWrite(VU_CLIP   , 0);    
  }
  else {                                // All green LEDs on + Clipping LED on
     digitalWrite(VU_PIN_1, 1);
     digitalWrite(VU_PIN_2, 1);
     digitalWrite(VU_PIN_3, 1);
     digitalWrite(VU_CLIP,    1);
  }
}
