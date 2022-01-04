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
 * @brief convert frequency to array index in spectrum
 * @param frequency frequency to convert in index
 */
int frequency_to_index(signal_t* signal, double frequency) {
    return round(frequency * signal->length / signal->sampling_frequency);
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
void compute_peak_list(signal_t* signal, double low_frequency, double high_frequency)
{
  erase_peak_list(signal->list);

  // find the PEAK_NUMBER highest peaks and add them to the list
  for (int i = 0; i < PEAK_NUMBER; i++) {
    peak_t peak = find_highest_peak(signal, low_frequency, high_frequency);
    add_peak(signal->list, &peak);
    erase_signal_at_peak(signal, &peak);
  }
}

/**
 * @brief find the highest peak of the spectrum 
 * @param signal struct containing signal info (the spectrum)
 * @param low_frequency the starting frequency of peak search
 * @param high_frequency the stoping frequency of peak search
 * @returns the highest peak of the spectrum
 */
peak_t find_highest_peak(signal_t* signal, double low_frequency, double high_frequency)
{  
  int low_index = frequency_to_index(signal, low_frequency);
  int high_index = frequency_to_index(signal, high_frequency);
  int max_index = low_index;

  double max_power = signal->real[low_index];

  for (int i = low_index + 1; i <= high_index; i++) {
    if (max_power < signal->real[i]) {
      max_power = signal->real[i]; 
      max_index = i;
    }
  }

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
  double interpolated_x;

  delta = (signal->real[index-1] - signal->real[index+1]) * 0.5 /
          (signal->real[index-1] - (2.0 * signal->real[index]) + signal->real[index+1]);

  interpolated_x = (index + delta) * signal->sampling_frequency / signal->length_with_padding;

  precise_peak.index = index;
  precise_peak.frequency = interpolated_x;
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