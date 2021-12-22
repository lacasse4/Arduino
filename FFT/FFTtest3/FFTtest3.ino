/*

  Example of use of the FFT libray to compute FFT for a signal sampled through the ADC.
        Copyright (C) 2018 Enrique Condés and Ragnar Ranøyen Homb

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "arduinoFFT.h"

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/
#define CHANNEL  A0
#define ALPHA    0.1
#define FREQ_GAP 0.1
#define ERASER_SIZE 7  // nombre impair

#define MAX_PEAK 5
typedef struct {
  double freq;
  double power;
} peak_t;

typedef struct {
  int nPeak;
  peak_t peak[MAX_PEAK];
} peakList_t;


const int samples = 128; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 500.0; //Hz, must be less than 10000 due to ADC

int sampling_period_us;
double alpha = 0.0;
double last = 0.0;
double shown = 0.0;

double vReal[samples];
double vImag[samples];
peakList_t list;

void setup()
{
  sampling_period_us = round(1000000.0/samplingFrequency);
  Serial.begin(250000);
  Serial.println("Ready");
}

void loop()
{
  getSignal(vReal, vImag, samples, sampling_period_us); 
  removeBias(vReal, samples);
  
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  /* Weigh data */
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */

  findPeakList(vReal, samples, samplingFrequency, &list); 

  if (list.nPeak == 0) {
    Serial.print("NO PEAK FOUND");
  }
  else {
    for (int i = 0; i < list.nPeak; i++) {
      Serial.print(list.peak[i].freq); 
      Serial.print("  ");
    }
  }
  Serial.println();
}

void getSignal(double *vReal, double *vImag, int samples, int sampling_period_us) 
{
  unsigned long microseconds = micros();
  for(int i = 0; i < samples; i++) {
      vReal[i] = analogRead(CHANNEL);
      vImag[i] = 0;
      while(micros() - microseconds < sampling_period_us);  // wait
      microseconds += sampling_period_us;
  }
}

void removeBias(double *vD, int samples) 
{
  double bias = 0.0;
  for (int i = 0; i < samples; i++) bias += vD[i];
  bias = bias / samples;
  for (int i = 0; i < samples; i++) vD[i] -= bias;
}

double findMaxPower(double *vD, int samples)
{
  double maxPower = 0.0;
  for (int i = 0; i < ((samples >> 1) + 1); i++) {
    if (maxPower < vD[i]) maxPower = vD[i]; 
  }
  return maxPower;
}

double findPower(double *vD, int samples) {
  double power = 0.0;
  for (int i = 0; i < ((samples >> 1) + 1); i++) power += vD[i];
  return power;  
}

void findFirstPeak(double *vD, int samples, double samplingFrequency, double *f, double *v)
{
  *f = -1.0;
  *v = -1.0;

  double maxi = findMaxPower(vD, samples);
  if (maxi < 100) return;
  
  double threshold = 0.2 * maxi;
  threshold = min(threshold, 60.0);
  
  double maxY = 0;  
  int iMax = 0;
  
  //If sampling_frequency = 2 * max_frequency in signal,
  //value would be stored at position samples/2
  for (int i = 1; i < (samples >> 1) + 1; i++) {
    boolean l1 = vD[i-1] < vD[i];
    boolean r1 = vD[i] > vD[i+1];
    boolean p1 = vD[i] > threshold;
    if (l1 && r1 && p1) {
      iMax = i;
      break;
    }
  }

  if (iMax == 0) return;
  
  double delta = 0.5 * ((vD[iMax - 1] - vD[iMax + 1]) / (vD[iMax - 1] - (2.0 * vD[iMax]) + vD[iMax + 1]));
  double interpolatedX = ((iMax + delta)  * samplingFrequency) / (samples - 1);

  // returned value: interpolated frequency peak apex
  *f = interpolatedX;
  *v = abs(vD[iMax - 1] - (2.0 * vD[iMax]) + vD[iMax + 1]);
}


void findPeakList(double *vD, int samples, double samplingFrequency, peakList_t *peakList)
{
  peakList->nPeak = 0;

  do {
    int iMax = findMajorPeakIndex(vD, samples);
    if (vD[iMax] < 100) return;

    addPrecisePeak(vD, samples, samplingFrequency, iMax, peakList);
    erasePeak(vD, samples, iMax);
  } 
  while (peakList->nPeak < MAX_PEAK);
}

unsigned int findMajorPeakIndex(double *vD, int samples)
{
  double maxPower = vD[0];
  int iMax = 0; 
  for (int i = 1; i < (samples >> 1) + 1; i++) {
    if (maxPower < vD[i]) {
      maxPower = vD[i];
      iMax = i;
    }
  }  
  return iMax;
}

void erasePeak(double *vD, int samples, int iMax)
{
  int index = iMax - (ERASER_SIZE) / 2;
  if (index < 0) index = 0;
  if ((index + ERASER_SIZE) > (samples >> 1)) index = (samples >> 1) - ERASER_SIZE; 
  
  for (int i = 0; i < ERASER_SIZE; i++) {
    vD[index++] = 0.0;
  }
}

void addPrecisePeak(double *vD, int samples, double samplingFrequency, int iMax, peakList_t *peakList)
{
  if (peakList->nPeak >= MAX_PEAK) return;
  double tmp = vD[iMax - 1] - (2.0 * vD[iMax]) + vD[iMax + 1];
  double delta = 0.5 * (vD[iMax - 1] - vD[iMax + 1]) / tmp;
  double interpolatedX = ((iMax + delta)  * samplingFrequency) / (samples - 1);
  peakList->peak[peakList->nPeak].freq = interpolatedX;
  peakList->peak[peakList->nPeak].power = abs(tmp);
  peakList->nPeak++;
}
