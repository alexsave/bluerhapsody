#ifndef FX_H
#define FX_H

#include "types.h"

typedef struct QuadVals {
    f64 x_minus_2;
    f64 x_minus_1;
    f64 y_minus_2;
    f64 y_minus_1;
} QuadVals;

void lpf(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q);
void naive_lowpass(f64* channel, u64 sample_count, f64 fraction);

#endif

