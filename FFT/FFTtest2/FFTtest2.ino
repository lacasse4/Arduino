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
#define CHANNEL A0
const uint16_t samples = 128; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 500; //Hz, must be less than 10000 due to ADC

unsigned int sampling_period_us;
unsigned long microseconds;

#define LOG
#define noLOOP

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

void setup()
{
  sampling_period_us = round(1000000*(1.0/samplingFrequency));
  Serial.begin(250000);
  Serial.println("Ready");
  delay(1000);
  Serial.println("Go!");
}

void loop()
{
  /*SAMPLING*/
  microseconds = micros();
  for(int i=0; i<samples; i++)
  {
      vReal[i] = analogRead(CHANNEL);
      vImag[i] = 0;
      while(micros() - microseconds < sampling_period_us){
        //empty loop
      }
      microseconds += sampling_period_us;
  }

  removeBias(vReal, samples);
  
  #ifdef LOG
  /* Print the results of the sampling according to time */
  Serial.println("Data:");
  PrintVector(vReal, samples, SCL_TIME);
  #endif
  
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  /* Weigh data */

  #ifdef LOG
  Serial.println("Weighed data:");
  PrintVector(vReal, samples, SCL_TIME);
  #endif
  
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
  
  #ifdef LOG
  Serial.println("Computed Real values:");
  PrintVector(vReal, samples, SCL_INDEX);
  Serial.println("Computed Imaginary values:");
  PrintVector(vImag, samples, SCL_INDEX);
  #endif

  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */

  #ifdef LOG
  Serial.println("Computed magnitudes:");
  PrintVector(vReal, samples, SCL_FREQUENCY);
  #endif

//  double x = FFT.MajorPeak(vReal, samples, samplingFrequency);

  double maxi = findMax(vReal, samples);

  double peakFreq;
  double peakPower;
  findFirstPeak(vReal, samples, samplingFrequency, &peakFreq, &peakPower); 

  Serial.print("FIRST PEAK = "); Serial.print(peakFreq, 2); 
  Serial.print("  PEAK POWER = "); Serial.print(peakPower, 2);
  Serial.print("  MAXIMUM = "); Serial.println(maxi, 2);
  
  #ifdef LOOP
//  delay(10); /* Repeat after delay */
  #else
  while(1); /* Run Once */
  #endif
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
  break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
  break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
  break;
    }
//    Serial.print(abscissa, 6);
//    if(scaleType==SCL_FREQUENCY)
//      Serial.print("Hz");
//    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}

void removeBias(double *signal, uint16_t size) 
{
  double bias = 0.0;
  for (uint16_t i = 0; i < size; i++) bias += signal[i];
  bias = bias / size;
  for (uint16_t i = 0; i < size; i++) signal[i] -= bias;
}

double findMax(double *vD, uint16_t samples)
{
  double maxi = 0.0;
  for (uint16_t i = 1; i < ((samples >> 1) + 1); i++) {
    if (maxi < vD[i]) maxi = vD[i]; 
  }
  return maxi;
}

double findPower(double *vD, uint16_t samples) {
  double power = 0.0;
  for (uint16_t i = 1; i < ((samples >> 1) + 1); i++) power += vD[i];
  return power;  
}

void findFirstPeak(double *vD, uint16_t samples, double samplingFrequency, double *f, double *v)
{
  *f = -1.0;
  *v = -1.0;

  double maxi = findMax(vD, samples);
  if (maxi < 100) return;
  
  double threshold = 0.2 * maxi;
  threshold = min(threshold, 60.0);
  
  double maxY = 0;  
  uint16_t iMax = 0;
  
  //If sampling_frequency = 2 * max_frequency in signal,
  //value would be stored at position samples/2
  for (uint16_t i = 1; i < (samples >> 1) + 1; i++) {
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

/*
void find3FirstPeaks(double *vD, uint16_t samples, double samplingFrequency, double *f, double *v)
{
  uint16_t IndexOfMaxY[3];
  
  for (int i = 0; i < 3; i++) {
    f[i] = -1.0;
    v[i] = -1.0;
    IndexOfMaxY[3] = 0;
  }

  double maxY = findMax(vD, samples) / 10;
  if (maxY < 20) return;
  
  
  //If sampling_frequency = 2 * max_frequency in signal,
  //value would be stored at position samples/2
  for (uint16_t i = 1; i < ((samples >> 1) + 1); i++) {
    if ((vD[i - 1] < vD[i]) && (vD[i] > vD[i + 1])) {
      if (vD[i] > maxY) {
        maxY = vD[i];
        IndexOfMaxY = i;
        
      }
    }
  }

  if (IndexOfMaxY == 0) return;
  
  double delta = 0.5 * ((vD[IndexOfMaxY - 1] - vD[IndexOfMaxY + 1]) / (vD[IndexOfMaxY - 1] - (2.0 * vD[IndexOfMaxY]) + vD[IndexOfMaxY + 1]));
  double interpolatedX = ((IndexOfMaxY + delta)  * samplingFrequency) / (samples - 1);
  //double popo =
  if (IndexOfMaxY == (samples >> 1)) //To improve calculation on edge values
    interpolatedX = ((IndexOfMaxY + delta)  * samplingFrequency) / (samples);
  // returned value: interpolated frequency peak apex
  *f = interpolatedX;
  *v = abs(vD[IndexOfMaxY - 1] - (2.0 * vD[IndexOfMaxY]) + vD[IndexOfMaxY + 1]);
}
*/
