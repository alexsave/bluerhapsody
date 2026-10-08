#ifndef SHAPE_H
#define SHAPE_H

#include <stdlib.h>

#include "types.h"


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

#endif

