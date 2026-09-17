#ifndef FFT_H
#define FFT_H

#include "types.h"

typedef struct Cpx {
    f64 re;
    f64 im;
} Cpx;

Cpx* fast(char* filename);
Cpx* fft(f64* channel, u32 sample_count, u32* bin_count);

#endif

