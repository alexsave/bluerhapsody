#ifndef SHAPE_H
#define SHAPE_H

#include <stdlib.h>

#include "types.h"

static const u8 SIN = 0;
static const u8 TRIANGLE_SUM = 1;
static const u8 TRIANGLE_NAIVE = 2;

static const u8 SQUARE_SUM = 3;
static const u8 SQUARE_NAIVE = 4;
static const u8 SQUARE_POLYBLEP = 5;
static const u8 SAW_SUM = 6;
static const u8 SAW_NAIVE = 7;
static const u8 SAW_POLYBLEP = 8;

static const u8 WHITE_NOISE = 9;
static const u8 BITCRUSH = 10;

void sf_reserve();
void sf_free();

typedef struct SinFast {
    f64 t;
    f64 i_n_minus_1;
    f64 i_n_minus_2;
} SinFast;

// malloc this from main lol
// shared scratch space for calculating sin(x), sin(2x), sin(3x),...
static SinFast * SF_SCRATCH;

f64 sawtooth(f64 phase, u16 levels);
f64 saw_naive(f64 phase);
f64 saw_polyblep(f64 phase);
f64 square(f64 phase, u16 levels);
f64 square_naive(f64 phase);
f64 square_polyblep(f64 phase);
f64 triangle(f64 phase, u16 levels);
f64 triangle_naive(f64 phase);
f64 sine(f64 phase);
f64 white(f64 phase);
f64 bitcrush(f64 phase);

#endif

