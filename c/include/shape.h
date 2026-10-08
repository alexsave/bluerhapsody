#ifndef SHAPE_H
#define SHAPE_H

#include <stdlib.h>

#include "types.h"



static const u8 TYPE_SIN = 0;
static const u8 TYPE_TRIANGLE = 1;
static const u8 TYPE_SQUARE = 2;
static const u8 TYPE_SAWTOOTH = 3;
static const u8 TYPE_WHITE_NOISE = 4;

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
f64 square(f64 phase, u16 levels);
f64 triangle(f64 phase, u16 levels);
f64 sine(f64 phase);
f64 white(f64 phase);

#endif

