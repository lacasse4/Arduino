#include <Signal.h>
#include <Spectrum.h>
#include <Peak.h>

/*
 * FFT Test program 6
 * Author: Vincent Lacasse
 * Date: 2021-03-14
 * 
 * Runs on Arduino Due + display on screen
 * Same version as FFT5 but using a C++ library
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
 * 
 * 
 * Note: I have found that the HT1632 produces noise that spills on the 
 * sound data acquisition circuit. The HT1632 has to be remove.
 */

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

#include "peak.h"
#include "signal.h"
#include "spectrum.h"


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
 * Allocate twice the size to signal arrays in order to perform "zero padding"
 */
const int nSampleFFT = nSample * 2;

/*
 * start and end indexes for frequency search
 */
const int startIndex = round((double)MIN_FREQUENCY * nSampleFFT / samplingFrequency);
const int stopIndex  = round((double)MAX_FREQUENCY * nSampleFFT / samplingFrequency);

Signal signal = Signal(signalLength, samplingFrequency);

// ICI - s'assure les longueurs pour le zero padding.

Spectrum spectrum = Spectrum(

// create FFT object
arduinoFFT FFT = arduinoFFT();

// use this line for single matrix
Adafruit_HT1632LEDMatrix matrix = Adafruit_HT1632LEDMatrix(HT_DATA, HT_WR, HT_CS);

void setup()
{
  Serial.begin(9600);

  matrix.begin(ADA_HT1632_COMMON_16NMOS);
  matrix.clearScreen();
  matrix.setRotation(0);
  matrix.setTextWrap(false);

}

void loop()
{
  double vReal[nSampleFFT];
  double vImag[nSampleFFT];
  peaklist_t list;
  peak_t fundamental;

  eraseSignal(vReal, nSampleFFT);
  eraseSignal(vImag, nSampleFFT);

  // acquire signal from ADC
  getSignal(vReal, nSample, samplingPeriod);

  // remove DC bias from the input signal
  removeBias(vReal, nSample);

  /* 
   * Compute the FFT of the input 
   * - Apply a Hamming window 
   * - compute the actual FFT with zero padding
   * - compute the magnitude (get the Spectrum)
   */
  FFT.Windowing(vReal, nSample, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, nSampleFFT, FFT_FORWARD); 
  FFT.ComplexToMagnitude(vReal, vImag, nSample);

  list = findPeakList(vReal, startIndex, stopIndex, nSampleFFT, samplingFrequency);
  fundamental = findFundamentalFrequency(list);

  displayPeak(fundamental);  
}

void displayPeak(peak_t peak) 
{
  char s[20];
  char t[20];
  sprintf(s, "%4.0f", peak.freq); 
  sprintf(t, "%4.0f", peak.power);
  matrix.clearScreen();
  matrix.setTextColor(0x1, 0);
  matrix.setCursor(0, 0);
  matrix.print(s);
  matrix.setCursor(0, 9);
  matrix.print(t);
  matrix.writeScreen();    
}


// --> to library
/**
 * @brief set an array values to 0.0 
 * @param vSignal array to erase
 * @param nSample number of sample erase
 */
void eraseSignal(double *vSignal, int nSample) 
{
  for (int i = 0; i < nSample; i++) {
    vSignal[i] = 0.0;
  }
}


// --> to library
/**
 * @brief get an analog signal from CHANNEL using the ADC
 * @param vSignal   array to store the analog signal
 * @param nSample  number of sample to digitize
 * @param samplingPeriod digitizer sampling period in micro seconds
 * @details vReal is filled with 'nSample' samples obtained from the ADC CHANNEL
 */
void getSignal(double *vSignal, int nSample, int samplingPeriod) 
{
  unsigned long microseconds = micros();

  for(int i = 0; i < nSample; i++) {
      vSignal[i] = analogRead(CHANNEL);
      while(micros() - microseconds < samplingPeriod);  // wait
      microseconds += samplingPeriod;
  }
}


// --> to library
/**
 * @brief remove the bias from an analog signal
 * @param vSignal  array containing the signal. Modified by this function.
 * @param nSample  number of sample to digitize
 */
void removeBias(double *vSignal, int nSample) 
{
  double bias = 0.0;

  // compute bias
  for (int i = 0; i < nSample; i++) {
    bias += vSignal[i];
  }

  // remove bias
  bias = bias / nSample;
  for (int i = 0; i < nSample; i++) {
    vSignal[i] -= bias;
  }
}

// --> to library
/**
 * @brief find the total power value of the signal
 * @param vSpectrum  array containing the spectrum
 * @param nSampleFFT  sample number of the full spectrum
 * @returns power value of the spectrum
 */
double findTotalPower(double *vSpectrum, int nSampleFFT) 
{
  double power = 0.0;

  for (int i = 0; i < (nSampleFFT >> 1); i++) {
    power += vSpectrum[i];
  }

  return power;  
}


// --> to library
/**
 * @brief find the highest peak of the spectrum 
 * @param vSpectrum  array containing the spectrum
 * @param nSampleFFT  sample number of the full spectrum
 * @returns the highest peak of the spectrum
 */
peak_t findHighestPeak(double *vSpectrum, int startIndex, int stopIndex, 
                       int nSampleFFT, double samplingFrequency)
{  
  int maxIndex = startIndex;
  double maxPower = vSpectrum[startIndex];

  for (int i = startIndex + 1; i <= stopIndex; i++) {
    if (maxPower < vSpectrum[i]) {
      maxPower = vSpectrum[i]; 
      maxIndex = i;
    }
  }

  return findPrecisePeak(vSpectrum, nSampleFFT, samplingFrequency, maxIndex);  
}


// --> to library
/**
 * @brief find the fisrt peak (lowest frequency) of the spectrum
 * @param vSpectrum  array containing the spectrum
 * @param nSampleFFT  sample number of the full spectrum
 * @param samplingFrequency  the signal sampling frequency
 * @returns the first peak encountered in the spectum.
 */
peak_t findFirstPeak(double *vSpectrum, int startIndex, int stopIndex, 
                     int nSampleFFT, double samplingFrequency)
{
  int index = startIndex;
  boolean left;
  boolean right;
  
  for (int i = startIndex + 1; i <= stopIndex; i++) {
    left = vSpectrum[i-1] < vSpectrum[i];
    right = vSpectrum[i] > vSpectrum[i+1];
    if (left && right) {
      index = i;
      break;
    }
  }

  return findPrecisePeak(vSpectrum, nSampleFFT, samplingFrequency, index);  
}


// --> to library
/**
 * @brief find the precise frequency of a peak given its index
 * @param vSpectrum  array containing the spectrum
 * @param nSampleFFT  sample number of the full spectrum
 * @param samplingFrequency  the signal sampling frequency
 * @returns the first peak encountered in the spectum.
 */
peak_t findPrecisePeak(double *vSpectrum, int nSampleFFT, double samplingFrequency, int index)
{
  peak_t precisePeak;

  double delta = (vSpectrum[index - 1] - vSpectrum[index + 1]) * 0.5 /
                 (vSpectrum[index - 1] - (2.0 * vSpectrum[index]) + vSpectrum[index + 1]);
  double interpolatedX = ((index + delta)  * samplingFrequency) / nSampleFFT;

  precisePeak.index = index;
  precisePeak.freq = interpolatedX;
  precisePeak.power = (vSpectrum[index - 1] + (2.0 * vSpectrum[index]) + vSpectrum[index + 1]) / 4;

  return precisePeak;
}


/**
 * @brief find the PEAK_NUMBER highest peaks from the vSpectrum array
 * @param vSpectrum  array containing the spectrum
 * @param nSampleFFT  sample number of the full spectrum
 * @param samplingFrequency  the signal sampling frequency
 * @details the list is in decreasing order of peak power 
 * @details (highest peak is at index 0). 
 * @details this function destroys the vSpectrum data
 */
peaklist_t findPeakList(double *vSpectrum, int startIndex, int stopIndex, 
                        int nSampleFFT, double samplingFrequency)
{
  peak_t peak;
  peaklist_t list = cleanList();

  // find the PEAK_NUMBER highest peaks and add them to the list
  for (int i = 0; i < PEAK_NUMBER; i++) {
    peak = findHighestPeak(vSpectrum, startIndex, stopIndex, nSampleFFT, samplingFrequency);
    list.peak[list.nPeak++] = peak;
    erasePeak(vSpectrum, nSampleFFT, peak);
  }

  return list;
}


/**
 * @brief initialize a peak list
 * @returns an empty peak list
 */
peaklist_t cleanList() 
{
  peaklist_t list;

  list.nPeak = 0;
  for (int i = 0; i < PEAK_NUMBER; i++) {
    list.peak[i].index = 0;
    list.peak[i].freq = 0;
    list.peak[i].power = 0;
  }
  return list;
}


/**
 * @brief erase (overwrite with 0) a peak from the vSpectrum array
 * @param vSpectrum  array containing the spectrum
 * @param nSampleFFT  sample number of the full spectrum
 * @param peak  the peak to erase form vSpectrum
 */
void erasePeak(double *vSpectrum, int nSampleFFT, peak_t peak)
{
  int index = peak.index - (ERASER_SIZE) / 2;
  int max = nSampleFFT >> 1;
  
  for (int i = 0; i < ERASER_SIZE && index < max; i++) {
    vSpectrum[index++] = 0.0;
  }
}


/**
 * @brief insert a peak in an ordrered list increasing by frequency
 * @param list  the peak list into which the peak must be inserted
 * @param peak  the peak to insert in the list 
 * @returns  the updated peak list  
 */
peaklist_t insertOrdreredPeak(peaklist_t list, peak_t peak) 
{
  int insertAt;

  if (list.nPeak >= PEAK_NUMBER) {
    return list;
  }
  
  // insert the first peak at location 0
  if (list.nPeak == 0) {
    list.peak[0] = peak;
    list.nPeak++; 
    return list;
  }

  // find where to insert in the list
  insertAt = list.nPeak;
  for (int i = 0; i < list.nPeak; i++) {
    if (list.peak[i].freq > peak.freq) {
      insertAt = i;
      break;
    }
  }

  // move old peaks to higher index values
  for (int j = list.nPeak; j > insertAt; j--) {
    list.peak[j] = list.peak[j-1];
  }
  
  // insert peak
  list.peak[insertAt] = peak;
  list.nPeak++;
  return list;
}

/**
 * @brief print peak data (for debbuging purpose)
 * @param peak  the peak to print 
 */
void printPeak(peak_t peak) {
  char s[100];
  sprintf(s, "peak: %3d, %4.0f, %4.0f", peak.index, peak.freq, peak.power);
  Serial.println(s);
}


/**
 * @brief find the fundamental frequency (if it exists) from the peak list
 * @param list  the peak list into which the fundamental is sought
 * @returns  the peak containing the frequency, or index = -1 if not found  
 */
/*
"The fundamental frequency of a signal is the greatest common divisor (GCD) 
of all the frequency components contained in a signal, and, equivalently, 
the fundamental period is the least common multiple (LCM) of all individual 
periods of the components." 
http://fourier.eng.hmc.edu/e59/lectures/Fundamental_Frequency/node1.html
*/
peak_t findFundamentalFrequency(peaklist_t list)
{
  peak_t fundamental;
  peak_t notFound;
  peaklist_t candidate = cleanList();
  peaklist_t divider = cleanList();
  peak_t highest = list.peak[0];

  notFound.index = -1;   // value to test
  notFound.freq  = 0.0;
  notFound.power = 0.0;

  if (list.nPeak < PEAK_NUMBER) {
    return notFound;
  }

  // make sure the highest peak is strong enough.
  // otherwise, there was not pitch in the sound recorded.
  if (highest.power < LOW_PEAK_POWER) {
    return notFound;
  }

  // find candidate peaks, that is:
  // peaks that have a lower frequency than the highest peak
  for (int i = 1; i < PEAK_NUMBER; i++) {
    if (list.peak[i].freq < highest.freq) { 
      candidate.peak[candidate.nPeak++] = list.peak[i];
    }
  }

  // if no candidates were found, then the highest peak has the fundamental
  if (candidate.nPeak == 0) {
    return highest;
  }

  // find divider peaks out of the candidate peak list.
  // a divider peak frequency is a divider of the highest peak frequency
  for (int i = 0; i < candidate.nPeak; i++) {
    double div = highest.freq / candidate.peak[i].freq;
    double remainder = fabs(div - round(div));
    if (remainder < SMALL_REMAINDER) {
      divider.peak[divider.nPeak++] = candidate.peak[i];
    }
  }

  // if no dividers where found, then the highest peak has the fundamental
  if (divider.nPeak == 0) {
    return highest;
  }

  // return the divider peak that has the lowest frequency
  fundamental = divider.peak[0];
  for (int i = 1; i < divider.nPeak; i++) {
    if (divider.peak[i].freq < fundamental.freq) {
      fundamental = divider.peak[i];
    }
  }

  return fundamental;
}
