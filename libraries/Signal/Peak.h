/**
 * Spectrum.h
 * 
 * Peak structure
 * 
 * Author: Vincent Lacasse (lacasse4@yahoo.com)
 * Date: 2021-02-28
 * Target system: Arduino Due
 * 
 */

typedef struct {
  int index;            // index of the peak
  double freq;          // frequency of the peak
  double power;         // power of the peak
} peak_t;
