#import <math.h>
#import <stdlib.h>

#import "shape.h"
#import "constants.h"
#import "types.h"
#import "rand.h"

// given phase, give me triangle wave
// and phase here is baseically note * 2 * M_PI * j/ SAMPLE_FREQUENCY already

// returns sin(phase)
f64 sf_init(f64 phase) {
    // which is faster? probably the same tbh
    //__sincos(phase, &(SF_SCRATCH->i_n_minus_1), &(SF_SCRATCH->t));
    SF_SCRATCH->i_n_minus_1 = sin(phase);
    SF_SCRATCH->t = cos(phase);

    SF_SCRATCH->t *= 2.0;
    SF_SCRATCH->i_n_minus_2 = 0.0;

    return SF_SCRATCH->i_n_minus_1;
}

f64 sf_next() {
    f64 next = (SF_SCRATCH->t * SF_SCRATCH->i_n_minus_1) - SF_SCRATCH->i_n_minus_2;
    SF_SCRATCH->i_n_minus_2 = SF_SCRATCH->i_n_minus_1;
    SF_SCRATCH->i_n_minus_1 = next;
    return next;
}

f64 sawtooth(f64 phase, u16 levels) {
    // this gets that first sin(x)
    f64 sum = sf_init(phase);

    for (u16 n = 2; n <= levels; n++) {
        sum += sf_next() / n;
    }

    return sum * -2.0 / M_PI;
}

f64 square(f64 phase, u16 levels) {
    f64 sum = sf_init(phase);
    for (u16 n = 3; n <= levels; n+=2) {
        sf_next(); // "2"
        sum += sf_next() / n; // "3"
    }
    return sum * 4.0 / M_PI;
}

f64 triangle(f64 phase, u16 levels) {
    f64 sum = sf_init(phase);
    for (u16 n = 3; n <= levels; n+=2) {
        sf_next();
        f64 unit = sf_next() / (n * n);
        if ((n&2) == 2) {
            sum -= unit;
        } else {
            sum += unit;
        }
    }
    return sum * 8.0 / (M_PI * M_PI);
}

f64 sine(f64 phase) {
    return sin(phase);
}

f64 white(f64 phase) {
    // this is super fucking cool btw
    //u64 p = phase;

    u64 p = *(u64*)(&phase);
    rand_next(&p);

    p &= MAX_U32;

    return (((f64)(2*p) / (f64)MAX_U32) - 1.0);
}

void sf_reserve() {
    SF_SCRATCH = malloc(sizeof(SinFast));
}

void sf_free() {
    free(SF_SCRATCH);
}
