/**
 * Signal.cpp
 * 
 * C++ library to perform general purpose signal processing tasks
 * 
 * Author: Vincent Lacasse (lacasse4@yahoo.com)
 * Date: 2021-02-28
 * Target system: Arduino Due
 * 
 */

#include "Signal.h"


/**
 * @brief Signal constructor
 * @param length signal's length
 * @param samplingFrequency signal's sampling frequency in Hertz
 * @details vSignal is twice the size for FFT zero padding
 */
Signal::Signal(int length, double samplingFrequency)
{
  this->length = length;
  this->samplingFrequency = samplingFrequency;
  this->samplingPeriod = round(1000000.0/samplingFrequency); // in microsec.
  vSignal = new double[length*2];
  FFT = new arduinoFFT();
  spectrum = new Spectrum(vSignal, length, samplingFrequency);
  erase();
}


/**
 * @brief Signal destructor
 */
Signal::~Signal()
{
  delete[] vSignal;
  delete FFT;
  delete spectrum;
}

double* Signal::getSignal()    { return vSignal; }
int Signal::getSignalLength()  { return length; }


/**
 * @brief digitizes an analog signal from channel using the Arduino ADC
 * @param channel ADC Arduino channel 
 * @details vReal is filled with 'lenght' samples obtained from the ADC CHANNEL
 */
void Signal::digitize(int channel) 
{
  unsigned long microseconds = micros();

  for(int i = 0; i < length; i++) {
      vSignal[i] = analogRead(channel);
      while(micros() - microseconds < samplingPeriod);  // wait
      microseconds += samplingPeriod;
  }
}


/**
 * @brief remove the bias from an analog signal
 */
void Signal::removeBias() 
{
  double bias = 0.0;

  // compute bias
  for (int i = 0; i < length; i++) {
    bias += vSignal[i];
  }

  // remove bias
  bias = bias / length;
  for (int i = 0; i < length; i++) {
    vSignal[i] -= bias;
  }
}


/**
 * @brief erases the signal
 */
void Signal::erase() 
{
  eraseArray(vSignal);
}


/**
 * @brief performs a FFT on the signal and return the spectrum
 * @brief the original signal is lost
 * @param windowType windowing mask (see arduinoFFT.h))
 * @details Windowing mask is set to Hamming
 */

Spectrum Signal::computeFFT(uint8_t windowType);
{
  double vImag[length*2];
  eraseArray(vImag);
  FFT.Windowing(vSignal, length, windowType, FFT_FORWARD);
  FFT.Compute(vSignal, vImag, length*2, FFT_FORWARD); 
  FFT.ComplexToMagnitude(vSignal, vImag, length);
  return spectrum;
}




// ---------------------------  PRIVATE METHODS ------------------------ 

/**
 * @brief set array values to 0.0 
 * @param vSignal array to erase
 */
void Signal::eraseArray(double *vSignal) 
{
  for (int i = 0; i < length*2; i++) {
    vSignal[i] = 0.0;
  }
}


/**
 * @brief copy an array 
 * @param dst destination array 
 * @param src source array
 */
void Signal::copyArray(double *dst, double* src) 
{
  for (int i = 0; i < length*2; i++) {
    dst[i] = src[i];
  }
}