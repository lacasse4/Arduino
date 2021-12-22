/**
 * Spectrum.cpp
 * 
 * C++ library for spectrum analysis
 * 
 * Author: Vincent Lacasse (lacasse4@yahoo.com)
 * Date: 2021-02-28
 * Target system: Arduino Due
 * 
 */

#include "Spectrum.h"


/**
 * @brief Spectrum class constructor
 */
Spectrum::Spectrum(double* vSpectrum, int length, double samplingFrequency)
{
    this->vSpectrum = vSpectrum;
    this->length = length;
    this->samplingFrequency = samplingFrequency;
    this->maxPeak = PEAK_NUMBER;
}


/**
 * @brief getters/setters
 */
double* Spectrum::getSpectrum()     { return vSpectrum; }
int Spectrum::getSpectrumLength()   { return length; }
peaklist_t Spectrum::getPeakList()  { return peakList; }


/**
 * @brief find the total power value of the signal
 * @returns power value of the spectrum
 */
double Spectrum::findTotalPower() 
{
  double power = 0.0;

  for (int i = 0; i < length; i++) {
    power += vSpectrum[i];
  }

  return power;  
}


/**
 * @brief find the highest peak of the spectrum 
 * @param lowFreq low frequency bound for search
 * @param highFreq high frequency bound for search 
 * @returns the highest peak of the spectrum
 */
peak_t Spectrum::findHighestPeak(double lowFreq, double highFreq)
{  
  int lowIndex = freqToIndex(lowFreq);
  int highIndex = freqToIndex(highFreq);
  int maxIndex = lowIndex;
  double maxPower = vSpectrum[lowIndex];

  for (int i = lowIndex + 1; i <= highIndex; i++) {
    if (maxPower < vSpectrum[i]) {
      maxPower = vSpectrum[i]; 
      maxIndex = i;
    }
  }

  return findPrecisePeak(maxIndex);  
}


/**
 * @brief find the fisrt peak (lowest frequency) of the spectrum
 * @param lowFreq low frequency bound for search
 * @param highFreq high frequency bound for search 
 * @returns the first peak encountered in the spectum.
 */
peak_t Spectrum::findFirstPeak(int lowFreq, int highFreq)
{
  int lowIndex = freqToIndex(lowFreq);
  int highIndex = freqToIndex(highFreq);
  int index = lowIndex;
  int left;
  int right;
  
  for (int i = lowIndex + 1; i <= highIndex; i++) {
    left = vSpectrum[i-1] < vSpectrum[i];
    right = vSpectrum[i] > vSpectrum[i+1];
    if (left && right) {
      index = i;
      break;
    }
  }

  return findPrecisePeak(index);  
}


/**
 * @brief find the precise frequency and power of peak given at index location
 * @param index  location index of the peak
 * @returns precice peak frequency and power in peak_t struct
 */
peak_t Spectrum::findPrecisePeak(int index)
{
  peak_t precisePeak;

  double delta = (vSpectrum[index - 1] - vSpectrum[index + 1]) * 0.5 /
                 (vSpectrum[index - 1] - (2.0 * vSpectrum[index]) + vSpectrum[index + 1]);
  double interpolatedX = ((index + delta)  * samplingFrequency) / length;

  precisePeak.index = index;
  precisePeak.freq = interpolatedX;
  precisePeak.power = (vSpectrum[index - 1] + (2.0 * vSpectrum[index]) + vSpectrum[index + 1]) / 4;

  return precisePeak;
}


/**
 * @brief find the highest peaks from the vSpectrum array (up to maxPeak)
 * @param lowFreq low frequency bound for search
 * @param highFreq high frequency bound for search 
 * @details the list is in decreasing order of peak power 
 * @details (highest peak is at index 0). 
 * @details this function destroys the vSpectrum data
 */
peaklist_t Spectrum::findPeakList(int lowFreq, int highFreq)
{
  peak_t peak;

  int lowIndex = freqToIndex(lowFreq);
  int highIndex = freqToIndex(highFreq);


  // find the highest peaks and add them to the list
  for (int i = 0; i < maxPeak; i++) {
    peak = findHighestPeak(lowIndex, highIndex);
    // add peak to list <---
    erasePeak(peak);
  }

  return peakList;
}



// ---------------------------  PRIVATE METHODS ------------------------

/**
 * @brief converts frequency to array index in spectrum
 * @param frequency frequency to convert in index
 */
int Spectrum::freqToIndex(double frequency) {
    return round((double)frequency * length / samplingFrequency);
}

/**
 * @brief initialize the peak list
 */
void Spectrum::cleanPeakList() 
{
  peakList.nPeak = 0;
  for (int i = 0; i < PEAK_NUMBER; i++) {
    peakList.peak[i].index = 0;
    peakList.peak[i].freq = 0;
    peakList.peak[i].power = 0;
  }
}

/**
 * @brief erase (overwrite with 0) a peak from the vSpectrum array
 * @param peak  the peak to erase form vSpectrum
 */
void Spectrum::erasePeak(peak_t peak)
{
  int index = peak.index - (ERASER_SIZE) / 2;
  
  for (int i = 0; i < ERASER_SIZE && index < length; i++) {
    vSpectrum[index++] = 0.0;
  }
}