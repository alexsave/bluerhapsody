#ifndef FFT_H
#define FFT_H

#include "types.h"

typedef struct Cpx {
    f64 re;
    f64 im;
} Cpx;

void fast(char* filename);

#endif

