
/*
 * FFT Test program 6
 * Author: Vincent Lacasse
 * Date: 2021-03-14
 * 
 * Runs on Arduino Due
 * Same version as FFT5 but using a C++ library.
 * HT1632 was removed; Frequency is sent to serial output.
 * 
 * Signal processing summary:
 * - the sound signal is sensed with a wide band (20Hz-20Khz) condenser 
 *   microphone
 * - the signal is amplified with a MAX4466 op-amp which brings it in 
 *   the 0-3.3V range
 * - the signal is filtered by an anti-alaising filter implemented with a 
 *   MAX7404 8th order low pass filter, cut off frequency of 870 Hz
 * - the signal is digitized with the Arduino Due ADC channel 0
 * 
 * Once the signal is in digital format, it is further processed by 
 * this program
 * - its bias is removed
 * - is is "zero padded" by placing it in a buffer twice as long as 
 *   the signal length
 * - it is windowed with a Hamming window
 * 
 * The signal is then converted to the frequency domain by a FFT.
 * and the magnitude of the frequency domain is computed (spectrum)
 * 
 * The 5 highest peaks are extracted from the spectrum's 66Hz-900Hz 
 * range. These peaks are processed in order to find the 
 * fundamental frequency which is a GDC of higher frequencies.
 * 
 * A fundamental frequency of 0 means that no fundamental was found.
 */

#include <arduinoFFT.h>
#include <spectrum.h>
#include <peak.h>
#include <peak_list.h>

/*
 * Signal processing defines
 */
#define CHANNEL          A0     // digitizer channel
#define ALPHA            0.1
#define FREQ_GAP         0.1
#define ERASER_SIZE      9      // must be an odd number
#define PEAK_NUMBER      5      // number of peaks to find in the spectrum
#define LOW_PEAK_POWER   1000   // lowest peak power considered in peak search
#define MAX_FREQUENCY    900    // maximum frequency detected (Hz)
#define MIN_FREQUENCY    66     // minimum frequency detected (Hz)
#define SMALL_REMAINDER  0.05   // remainder used to validate a GCD 

#define LOOP

/* 
 * In order to eliminate signal alaising during the digitization process,
 * the signal sampling frequency must be at least twice as much as the maximum
 * frequency present in the analog signal (as per Nyqvist rate).
 * Here, we use a 4000 Hz sampling frequency. This is 4 times greater that 
 * the analog signal frequency which is limited by the analog low pass filter
 * with a cut off frequency of 870 Hz
 */
const double samplingFrequency = 4000.0; // in Hz, must be less than 10000 Hz

/*
 * With a sampling frequency of 4000 Hz and a sample number 1024, the minimum
 * frequency that can be detected is approximatly 8 Hz. This is low enough 
 * to detect the minimum audible frequency which is approx. 20 Hz
 */
const int signalLength = 1024;

/*
 * Signal object
 */

 signal_t* sig;


void setup()
{
  sig = create_signal(signalLength, samplingFrequency, ZERO_PADDING_ENABLED);
  Serial.begin(9600);
  Serial.println("Ready");
}

void loop()
{
  peak_t fundamental;

  // acquire signal from ADC
  acquire(sig, CHANNEL);

  // compute spectrum
  compute_spectrum(sig);

  // compute peak list
  compute_peak_list(sig, MIN_FREQUENCY, MAX_FREQUENCY);

  // find fundamental frequency  
  fundamental = find_fundamental_frequency(get_peak_list(sig));

  displayPeak(fundamental);  
}

void displayPeak(peak_t peak) 
{
    char s[100];
    sprintf(s, "F: %03.0f  ", peak.frequency); Serial.print(s);
    Serial.println();
}


/**
 * @brief print peak data (for debbuging purpose)
 * @param peak  the peak to print 
 */
void printPeak(peak_t peak) {
  char s[100];
  sprintf(s, "peak: %3d, %4.0f, %4.0f", peak.index, peak.frequency, peak.power);
  Serial.println(s);
}
