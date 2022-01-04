/**
 * peak_list.h
 * 
 * C module representing a peak_list 
 * 
 * Author: Vincent Lacasse (lacasse4@yahoo.com)
 * Date: 2021-12-23
 * Target system: Arduino Due
 * 
 */

#ifndef _PEAK_LIST_H
#define _PEAK_LIST_H

#define PEAK_NUMBER       5      // default number of peaks to find in the spectrum
#define LOWEST_PEAK_POWER 1000   // lowest peak power considered in peak search
#define SMALL_REMAINDER   0.05   // remainder used to validate a GCD 


#include "peak.h"

typedef struct peak_list peak_list_t;

peak_list_t* create_peak_list();
void delete_peak_list(peak_list_t* peak_list);
void erase_peak_list(peak_list_t* peak_list);
int add_peak(peak_list_t* peak_list, peak_t* peak);
peak_t get_peak(peak_list_t* peak_list, int index);
int list_size(peak_list_t* peak_list);
peak_t find_fundamental_frequency(peak_list_t* peak_list);

#endif