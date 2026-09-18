#ifndef WASM_H
#define WASM_H

#include "types.h"

typedef struct FreqMag {
    f64 freq;
    f64 mag;
} FreqMag;

// return type for FFT display
typedef struct Spectra {
    // across entire file
    u64 sample_count; 

    u64 sample_rate; //48khz
    // 0 for DFT over entire lenght, but this will usually be 8192 or so
    u64 window_samples; 
    u64 num_windows; // duration * smaple rate / window_samples * 2 ish

    // first "window_samples" f64s are frequency
    // then num_windows x window_samples f64 of amplitudes
    f64 data[];
} Spectra;

// if you request sample size 8192
// we need to return a list of frequencies, so 8192/48000, 2*8192/48000, etc.
// num frequencies
// num of how many amplitude "bars" we have, the longer the audio is, the more we have
// and return sample rate 48khz
// and sample count
// duration


#endif

