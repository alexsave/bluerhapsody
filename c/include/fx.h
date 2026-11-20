#ifndef FX_H
#define FX_H

#include "types.h"

typedef struct QuadVals {
    f64 x_minus_2;
    f64 x_minus_1;
    f64 y_minus_2;
    f64 y_minus_1;
} QuadVals;

f64 lpf_one(QuadVals * quad_vals, f64 x, f64 freq0, f64 q);

void lpf(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q);
void hpf(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q);
void bpf(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q);
void bpf0(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q);
void notch(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q);
void apf(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q);
void peaking_eq(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q, f64 A);
void low_shelf(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q, f64 A);
void high_shelf(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q, f64 A);

void naive_lowpass(f64* channel, u64 sample_count, f64 fraction);

#endif

