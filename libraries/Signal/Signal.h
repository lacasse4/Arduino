/**
 * Signal.h
 * 
 * C++ library to perform general purpose signal processing tasks
 * 
 * Author: Vincent Lacasse (lacasse4@yahoo.com)
 * Date: 2021-02-28
 * Target system: Arduino Due
 * 
 */


#ifndef _SIGNAL
#define _SIGNAL

#include "Arduino.h"
#include "../arduinoFFT/src/arduinoFFT.h"
#include "Spectrum.h"

class Signal {
  public:
    Signal(int length, double samplingFrequency);
    ~Signal();
    double* getSignal();
    int getSignalLength();

    void digitize(int channel);
    void removeBias();
    void erase();
    Spectrum computeFFT(uint8_t windowType);

  private:
    int length;                 // number of samples
    double samplingFrequency;   // samplingFrequency in Hertz
    int samplingPeriod;         // sampling period in microseconds
    double* vSignal;            // signal array
    arduinoFFT* FFT;             // FFT object from ArduinoFFT lib
    Spectrum* spectrum;          // signal spectrum
  
    void eraseArray(double* array);
    void copyArray(double* dst, double* src);
};

#endif