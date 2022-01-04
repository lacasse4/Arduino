/**
 * peak_list.c
 * 
 * C module to perform peak processing on a signal
 * 
 * Author: Vincent Lacasse (lacasse4@yahoo.com)
 * Date: 2021-12-23
 * Target system: Arduino Due
 * 
 */

#include <stdlib.h>
#include <math.h>
#include "peak_list.h"
#include "peak.h"

struct peak_list  {
    int n_peak;                  // number of peak found in signal
    peak_t peak[PEAK_NUMBER];    // list of peaks found
};

/**
 * @brief create at peak list
 */
peak_list_t* create_peak_list() 
{
  peak_list_t* list = (peak_list_t*) malloc(sizeof(peak_list_t));
  if (list == NULL) return NULL;

  erase_peak_list(list);
  return list;
}

/**
 * @brief release peak_list resources
 */
void delete_peak_list(peak_list_t* list) 
{
  free(list);
}

/**
 * @brief erase the peak list
 */
void erase_peak_list(peak_list_t* list) 
{
  list->n_peak = 0;
  for (int i = 0; i < PEAK_NUMBER; i++) {
    erase_peak(&list->peak[i]);
  }
}

/**
 * @brief return the list's size
 */
int list_size(peak_list_t* list) 
{
  return list->n_peak;
}

/**
 * @brief add a peak to the list
 * @param peak the peak to add to the list
 * @return 1 if OK, 0 if list was already full
 */
int add_peak(peak_list_t* list, peak_t* peak) 
{
  if (list->n_peak >= PEAK_NUMBER) return 0;
  list->peak[list->n_peak++] = *peak;
  return 1;
}

/**
 * @brief return peak at index
 * @param index index of the peak sought
 * @return a peak
 */
peak_t get_peak(peak_list_t* list, int index) 
{
  return list->peak[index];
}

/**
 * @brief find the fundamental frequency (if it exists) from the peak list
 * @param list  the peak list into which the fundamental is sought
 * @returns  the peak containing the frequency, or index = -1 if not found  
 */
/*
"The fundamental frequency of a signal is the greatest common divisor (GCD) 
of all the frequency components contained in a signal, and, equivalently, 
the fundamental period is the least common multiple (LCM) of all individual 
periods of the components." 
http://fourier.eng.hmc.edu/e59/lectures/Fundamental_Frequency/node1.html
*/
peak_t find_fundamental_frequency(peak_list_t* list)
{
  peak_t fundamental;
  peak_t not_found;
  peak_list_t candidate;
  peak_list_t divider;
  peak_t highest = get_peak(list, 0);

  erase_peak_list(&candidate);
  erase_peak_list(&divider);
  erase_peak(&not_found);  // value to test
  
  if (list_size(list) < PEAK_NUMBER) {
    return not_found;
  }

  // make sure the highest peak is strong enough.
  // otherwise, there was not pitch in the sound recorded.
  if (highest.power < LOWEST_PEAK_POWER) {
    return not_found;
  }

  // find candidate peaks, that is:
  // peaks that have a lower frequency than the highest peak
  for (int i = 1; i < PEAK_NUMBER; i++) {
    if (get_peak(list, i).frequency < highest.frequency) { 
      add_peak(&candidate, &list->peak[i]);
    }
  }

  // if no candidates were found, then the highest peak is the fundamental
  if (list_size(&candidate) == 0) {
    return highest;
  }

  // find divider peaks out of the candidate peak list.
  // a divider peak frequency is a divider of the highest peak's frequency
  for (int i = 0; i < list_size(&candidate); i++) {
    peak_t under_test = get_peak(&candidate, i);
    double div = highest.frequency / under_test.frequency;
    double remainder = fabs(div - round(div));
    if (remainder < SMALL_REMAINDER) {
      add_peak(&divider, &under_test);
    }
  }

  // if no dividers where found, then the highest peak is the fundamental
  if (list_size(&divider) == 0) {
    return highest;
  }

  // return the divider peak that has the lowest frequency
  fundamental = get_peak(&divider, 0);
  for (int i = 1; i < list_size(&divider); i++) {
    peak_t under_test = get_peak(&divider, i);
    if (under_test.frequency < fundamental.frequency) {
      fundamental = under_test;
    }
  }

  return fundamental;
}
