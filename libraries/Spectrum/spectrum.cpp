/**
 * spectrum.c
 * 
 * C library to perform general purpose signal processing tasks
 * 
 * Author: Vincent Lacasse (lacasse4@yahoo.com)
 * Date: 2021-02-28
 * Target system: Arduino Due
 * 
 */


#include <stdlib.h>
#include "spectrum.h"
#include "peak.h"
#include "peak_list.h"

/*
extern char ttt[3][100];
extern char uuu[3][100];
int uuu_c;
*/

/*
 * Signal structure
 */
struct signal {
    int length;                 // number of samples
    double sampling_frequency;  // samplingFrequency in Hertz
    int sampling_period;        // sampling period in microseconds

    int length_with_padding;    // number of samples with zero padding
    int zero_padding_enabled;   // indicates if enough memory is allocated for zero padding
    unsigned char window_type;  // window type for pre FFT windowing

    double* real;               // signal array (real part)
    double* imag;               // signal array (imaginary part)
    peak_list_t* list;          // spectrum's peak list
};

/**
 * @brief create a signal
 * @param length signal's length
 * @param sampling_frequency signal's sampling frequency in Hertz
 * @param zero_padding_enabled extra memory allocated for signal if true
 * @details signal is twice the 'length' if FFT zero padding is enabled
 */
signal_t* create_signal(int length, double sampling_frequency, int zero_padding_enabled)
{
  signal_t* signal = (signal_t*) malloc(sizeof(signal_t));
  if (signal == NULL) return NULL;

  signal->length = length;
  signal->sampling_frequency = sampling_frequency;
  signal->zero_padding_enabled = zero_padding_enabled;
  signal->length_with_padding = length * (zero_padding_enabled ? 2 : 1);
  signal->sampling_period = round(CYCLES_PER_MHZ/sampling_frequency); // in microsec.

  signal->window_type = FFT_WIN_TYP_HAMMING;

  // Allocate twice the memory if zero padding is enabled.
  // This allows to perform zero padding safely.
  signal->real = (double *) malloc(signal->length_with_padding * sizeof(double));
  signal->imag = (double *) malloc(signal->length_with_padding * sizeof(double));
  signal->list = create_peak_list();

  if (signal->imag == NULL || signal->imag == NULL || signal->list == NULL) {
    delete_peak_list(signal->list);
    free(signal->imag);
    free(signal->real);
    free(signal);
    return NULL;
  }

  return signal;
}

/**
 * @brief release signal resources
 */
void delete_signal(signal_t* signal)
{
  free(signal->imag);
  free(signal->real);
  free(signal);
}

/**
 * @brief getters
 */
int get_length(signal_t* signal)                { return signal->length; }
int get_length_with_padding(signal_t* signal)   { return signal->length_with_padding; }
int is_padding_enabled(signal_t* signal)        { return signal->zero_padding_enabled; }
double get_sampling_frequency(signal_t* signal) { return signal->sampling_frequency; }
double* get_signal_array(signal_t* signal)      { return signal->real; }
peak_list_t* get_peak_list(signal_t* signal)    { return signal->list; }

/**
 * @brief digitizes an analog signal from channel using the Arduino ADC
 * @param channel ADC Arduino channel 
 * @param removeBias bias is removed after data aquisition if true
 * @details signal is filled with 'length' samples obtained from the ADC CHANNEL
 */
void acquire(signal_t* signal, int channel) 
{
  unsigned long microseconds;

  erase_array(signal->real, signal->length_with_padding);
  erase_array(signal->imag, signal->length_with_padding);

  microseconds = micros();
  for(int i = 0; i < signal->length; i++) {
      signal->real[i] = analogRead(channel);
      while(micros() - microseconds < signal->sampling_period);  // wait
      microseconds += signal->sampling_period;
  }

  remove_bias(signal);
}

/**
 * @brief remove the bias from an analog signal (real part only)
 */
void remove_bias(signal_t* signal) 
{
  double bias = 0.0;

  // compute bias
  for (int i = 0; i < signal->length; i++) {
    bias += signal->real[i];
  }

  // remove bias
  bias = bias / signal->length;
  for (int i = 0; i < signal->length; i++) {
    signal->real[i] -= bias;
  }
}


/**
 * @brief set the window type for FFT processing 
 * @param window_type window type from arduinoFFT lib
 */
void set_window_type(signal_t* signal, unsigned char window_type) {
  signal->window_type = window_type;
}

/**
 * @brief compute signal spectrum using arduinoFFt lib.
 * @details the original time domain signal is overwritten
 */
void compute_spectrum(signal_t* signal) 
{
  arduinoFFT FFT;
  FFT.Windowing(signal->real, signal->length, signal->window_type, FFT_FORWARD);
  FFT.Compute(signal->real, signal->imag, signal->length_with_padding, FFT_FORWARD); 
  FFT.ComplexToMagnitude(signal->real, signal->imag, signal->length);
}

/**
 * @brief convert frequency to spectrum array index
 * @param frequency frequency to be converted to index
 */
int frequency_to_index(signal_t* signal, double frequency) {
  return round(frequency * signal->length_with_padding / signal->sampling_frequency);
}

/**
 * @brief convert spectrum array index to frequency
 * @param index index to be converted to a frequency
 */
double index_to_frequency(signal_t* signal, double index) {
  return index * signal->sampling_frequency / signal->length_with_padding;
}

/**
 * @brief erase (overwrite with 0) a peak from the signal array (real part)
 * @param peak the peak to erase form the spectrum
 * @details it is assumed that the signal array contains the spectrum at this point
 */
void erase_signal_at_peak(signal_t* signal, peak_t* peak)
{
  int index = peak->index - (ERASER_SIZE) / 2;
  index = index < 0 ? 0 : index;

  for (int i = 0; i < ERASER_SIZE && index < signal->length; i++) {
    signal->real[index++] = 0.0;
  }
}

/**
 * @brief find the PEAK_NUMBER highest peaks from the signal
 * @param low_frequency the starting frequency of peak search
 * @param high_frequency the stoping frequency of peak search
 * @details the list is in decreasing order of peak power 
 * @details (highest peak is at index 0). 
 * @details this function modifies the spectrum data
 */
/*
"The fundamental frequency of a signal is the greatest common divisor (GCD) 
of all the frequency components contained in a signal, and, equivalently, 
the fundamental period is the least common multiple (LCM) of all individual 
periods of the components." 
http://fourier.eng.hmc.edu/e59/lectures/Fundamental_Frequency/node1.html
*/

#include <stdio.h>

void compute_peak_list(signal_t* signal, double low_frequency, double high_frequency)
{
  peak_t highest;
  double candidate_frequency;
  double candidate_low;
  double candidate_high;
  peak_t peak_found;

  /*
  int i = 0;

  strcpy(ttt[0], "null");
  strcpy(ttt[1], "null");
  strcpy(ttt[2], "null");

  strcpy(uuu[0], "null");
  strcpy(uuu[1], "null");
  strcpy(uuu[2], "null");
  uuu_c = 0;
  */

  erase_peak_list(signal->list);

  // find highest peak in entire frequency range
  highest = find_highest_power(signal, low_frequency, high_frequency);
  if (highest.power < LOWEST_PEAK_POWER) return;
  add_peak(signal->list, &highest);

  // check if peaks exist in the spectrum at lower frequencies that
  // are exact dividers of highest peak's frequency
  for (int frequency_divider = 2; frequency_divider <= 4; frequency_divider++) {

    // candidate frequency is an exact divider of the highest peak's frequency
    candidate_frequency = highest.frequency / frequency_divider;
    if (candidate_frequency < low_frequency) return;

    // define a search window where the peak will be sought
    candidate_low = candidate_frequency - candidate_frequency * SEARCH_WINDOW;
    candidate_high = candidate_frequency + candidate_frequency * SEARCH_WINDOW;
    peak_found = find_highest_power(signal, candidate_low, candidate_high);

    // if a peak was found in the search window, make sure its power is sufficient
    if (peak_found.index != -1) {
      if (peak_found.power > highest.power * HIGHEST_RATIO) {
        add_peak(signal->list, &peak_found);
      }
    }
    /*
    sprintf(ttt[i++], "cf = %lf, pfi = %d, pfp= %lf, hp = %lf", 
        candidate_frequency, peak_found.index, peak_found.power, highest.power);
    */
  }
}


/**
 * @brief find the highest peak of spectrum in specified frequency range
 * @param signal struct containing signal info (the spectrum)
 * @param low_frequency the starting frequency of peak search
 * @param high_frequency the stoping frequency of peak search
 * @returns the highest peak of the spectrum
 */
peak_t find_highest_power(signal_t* signal, double low_frequency, double high_frequency)
{  
  int low_index;
  int high_index;
  int max_index;
  double max_power;
  int is_max;

  // to be returned if a peak is not found
  peak_t not_found;
  erase_peak(&not_found);

  low_index = frequency_to_index(signal, low_frequency);
  if (low_index <= 0) return not_found;

  high_index = frequency_to_index(signal, high_frequency);
  if (high_index >= signal->length-1) return not_found;

  // find max peak in specified frequency range
  max_index = low_index;
  max_power = signal->real[low_index];
  for (int i = low_index; i <= high_index; i++) {
    if (max_power < signal->real[i]) {
      max_power = signal->real[i]; 
      max_index = i;
    }
  }

  /*
  sprintf(uuu[uuu_c++], "li = %d, hi = %d, mi = %d", low_index, high_index, max_index);
  */

  // there is no peak if max_index is a boundary value
  if (max_index == low_index) return not_found;
  if (max_index == high_index) return not_found;

  // power at max_index must be surrounded by lower power values
  is_max = signal->real[max_index] > signal->real[max_index-1] &&
           signal->real[max_index] > signal->real[max_index+1];
  if (!is_max) return not_found;

  return find_precise_peak(signal, max_index);  
}

/**
 * @brief find the precise frequency of a peak given its index
 * @param signal struct containing signal info (the spectrum)
 * @param index index of the maximum peak within the spectrum
 * @returns a peak with precise frequency
 */
peak_t find_precise_peak(signal_t* signal, int index)
{
  peak_t precise_peak;
  double delta;
  double interpolated_frequency;

  delta = (signal->real[index-1] - signal->real[index+1]) * 0.5 /
          (signal->real[index-1] - (2.0 * signal->real[index]) + signal->real[index+1]);

  interpolated_frequency = index_to_frequency(signal, index + delta);

  precise_peak.index = index;
  precise_peak.frequency = interpolated_frequency;
  precise_peak.power = (signal->real[index-1] + (2.0 * signal->real[index]) + signal->real[index+1]) / 4;

  return precise_peak;
}


/**
 * @brief set array values to 0.0 
 * @param array pointer to first value to erase
 * @param length number of value to erase
 */
void erase_array(double *array, int length) 
{
  for (int i = 0; i < length; i++) {
    array[i] = 0.0;
  }
}


/**
 * @brief copy scr array to dst 
 * @param dst destination array 
 * @param src source array
 * @param length number of samples to transfer
 */
void copy_array(double *dst, double* src, int length) 
{
  for (int i = 0; i < length; i++) {
    dst[i] = src[i];
  }
}