/**
 * spectrum.h
 * 
 * C library to perform general purpose signal processing tasks
 * 
 * Author: Vincent Lacasse (lacasse4@yahoo.com)
 * Date: 2021-02-28
 * Target system: Arduino Due
 * 
 */


#ifndef _SPECTRUM_H
#define _SPECTRUM_H

#include "spectrum.h"
#include "peak_list.h"
#include "peak.h"
#include "arduinoFFT.h"

#define ZERO_PADDING_ENABLED  1
#define ZERO_PADDING_DISABLED 0
#define ERASER_SIZE           9       // must be an odd number
#define CYCLES_PER_MHZ        1000000.0

typedef struct signal signal_t;

signal_t* create_signal(int length, double sampling_frequency, int zero_padding_enabled);
void delete_signal(signal_t* signal);

int get_length(signal_t* signal);
int get_length_with_padding(signal_t* signal);
int is_padding_enabled(signal_t* signal);
double get_sampling_frequency(signal_t* signal);
double* get_signal_array(signal_t* signal);
peak_list_t* get_peak_list(signal_t* signal);

void set_window_type(signal_t* signal, unsigned char window_type);
  
void acquire(signal_t* signal, int channel);
void compute_spectrum(signal_t* signal);

void remove_bias(signal_t* signal);
void erase_signal(signal_t* signal);
void erase_signal_at_peak(signal_t* signal, peak_t* peak);

void compute_peak_list(signal_t* signal, double low_frequency, double high_frequency);
peak_t find_highest_peak(signal_t* signal, double low_frequency, double high_frequency);
peak_t find_precise_peak(signal_t* signal, int index);

void erase_array(double *array, int length);
void copy_array(double *dst, double* src, int length);

#endif