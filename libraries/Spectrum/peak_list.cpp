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

peak_t find_fundamental_frequency(peak_list_t* list)
{
  peak_t fundamental;
  peak_t not_found;

  erase_peak(&not_found);  // value to test
  
  if (list_size(list) == 0) {
    return not_found;
  }

  if (list_size(list) == 1) {
    return get_peak(list, 0);
  }

  if (list_size(list) == 2) {
    return get_peak(list, 1);
  }

  if (list_size(list) == 3) {
    return get_peak(list, 2);
  }

  if (list_size(list) == 4) {
    return get_peak(list, 3);
  }

  return not_found;
}
