/**
 * peak.h
 * 
 * C module representing a peak in a signal
 * 
 * Author: Vincent Lacasse (lacasse4@yahoo.com)
 * Date: 2021-12-23
 * Target system: Arduino Due
 * 
 */

#ifndef _PEAK_H
#define _PEAK_H

typedef struct peak {
    int index;            // peak index in signal array
    double frequency;     // peak frequency
    double power;         // peak power
} peak_t;

void erase_peak(peak_t* peak);

#endif