/**
 * Spectrum.h
 * 
 * C++ library for spectrum processing
 * 
 * Author: Vincent Lacasse (lacasse4@yahoo.com)
 * Date: 2021-02-28
 * Target system: Arduino Due
 * 
 */


#ifndef _SPECTRUM
#define _SPECTRUM

#define PEAK_NUMBER   5   // default number of peaks to find in the spectrum
#define ERASER_SIZE   9   // must be an odd number

#include <cmath>
#include "Peak.h"


typedef struct {
  int nPeak;
  peak_t peak[PEAK_NUMBER];
} peaklist_t;


class Spectrum {
  public:
    Spectrum(double* vSpectrum, int length, double samplingFrequency);
    ~Spectrum();

    double* getSpectrum();
    int getSpectrumLength();
    peaklist_t getPeakList();
    double findTotalPower();
    peak_t findHighestPeak(double lowFreq, double highFreq);
    peak_t findFirstPeak(int lowFreq, int highFreq);
    peak_t findPrecisePeak(int index);
    peaklist_t findPeakList(int lowFreq, int highFreq);



  private:
    int length;                 // number of samples in spectrum
    int samplingFrequency;      // in Hertz
    double* vSpectrum; 
    int maxPeak;
    peaklist_t peakList;  

    int freqToIndex(double frequency);
    void cleanPeakList();
    void erasePeak(peak_t peak);

};

#endif
