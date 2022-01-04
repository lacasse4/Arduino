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

#include "peak.h"

/**
 * @brief initialize a peak
 */
void erase_peak(peak_t* peak) {
    peak->index = -1;
    peak->frequency = 0.0;
    peak->power = 0.0;
};