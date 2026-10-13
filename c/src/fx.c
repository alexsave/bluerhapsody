#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "fx.h"
#include "types.h"
#include "constants.h"

// this will take the address of the channel and set it to a new array
void _biquad(QuadVals * quad_vals, f64* x, u64 sample_count, f64 a0, f64 a1, f64 a2, f64 b0, f64 b1, f64 b2) {

    a1 /= a0;
    a2 /= a0;
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;

    f64* y = malloc(sizeof(f64) * sample_count);

    // what the hell do we set y[0] and y[1] to?
    // 0?

    //y[0] = 0.0;
    //y[1] = 0.0;

    //y[0] = b0*x[0];
    //y[1] = b0*x[1] + b1*x[0] - a1*y[0];

    y[0] = b0*x[0] + b1*quad_vals->x_minus_1 + b2*quad_vals->x_minus_2 - a1*quad_vals->y_minus_1 - a2*quad_vals->y_minus_2;
    y[1] = b0*x[1] + b1*x[0] + b2*quad_vals->x_minus_1 - a1*y[0] - a2*quad_vals->y_minus_1;

    for (u64 i = 2; i < sample_count; i++) {
        // see if it is worth keeping the last 5 in local vars
        y[i] = b0*x[i] + b1*x[i-1] + b2*x[i-2] - a1*y[i-1] - a2*y[i-2];
    }

    // we COULD do this in one pass, setting x[i-3] as we go?

    //memcpy(x, y, sample_count * sizeof(f64));

    for (u64 i = 0; i < sample_count;i++) {
        x[i] = y[i];
    }
    
    quad_vals->x_minus_2 = x[sample_count-2];
    quad_vals->x_minus_1 = x[sample_count-1];
    quad_vals->y_minus_2 = y[sample_count-2];
    quad_vals->y_minus_1 = y[sample_count-1];

    free(y);
    //*channel = y;
}

void lpf(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q) {
    f64 w0 = TWO_PI * freq0 / SAMPLE_RATE;
    
    f64 sinw0 = sin(w0);
    f64 cosw0 = cos(w0);

    f64 b1 = 1.0 - cosw0;
    f64 b0 = b1 / 2.0;
    f64 b2 = b1 / 2.0;
    f64 a1 = -2.0 * cosw0;

    f64 alpha = sinw0/(2.0*q);
    f64 a0 = 1.0 + alpha;
    f64 a2 = 1.0 - alpha;
   
    //printf("%f, %f, %f, %f, %f, %f\n", a0, a1, a2, b0, b1, b2);

    _biquad(quad_vals, channel, sample_count, a0, a1, a2, b0, b1, b2);
}

void hpf(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q) {
    f64 w0 = TWO_PI * freq0 / SAMPLE_RATE;
    
    f64 sinw0 = sin(w0);
    f64 cosw0 = cos(w0);

    f64 b1 = -1.0*(1.0 + cosw0);
    f64 b0 = b1 / -2.0;
    f64 b2 = b1 / -2.0;
    f64 a1 = -2.0 * cosw0;

    f64 alpha = sinw0/(2.0*q);
    f64 a0 = 1.0 + alpha;
    f64 a2 = 1.0 - alpha;
   
    _biquad(quad_vals, channel, sample_count, a0, a1, a2, b0, b1, b2);
}

// constant skirt gain
void bpf(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q) {
    f64 w0 = TWO_PI * freq0 / SAMPLE_RATE;
    
    f64 sinw0 = sin(w0);
    f64 cosw0 = cos(w0);

    f64 b1 = 0;
    f64 b0 = sinw0 / 2.0;
    f64 b2 = -b0;
    f64 a1 = -2.0 * cosw0;

    f64 alpha = sinw0/(2.0*q);
    f64 a0 = 1.0 + alpha;
    f64 a2 = 1.0 - alpha;
   
    _biquad(quad_vals, channel, sample_count, a0, a1, a2, b0, b1, b2);
}

// 0db peak gain
void bpf0(QuadVals * quad_vals, f64* channel, u64 sample_count, f64 freq0, f64 q) {
    f64 w0 = TWO_PI * freq0 / SAMPLE_RATE;
    
    f64 sinw0 = sin(w0);
    f64 cosw0 = cos(w0);

    f64 alpha = sinw0/(2.0*q);
    f64 b1 = 0;
    f64 b0 = alpha;
    f64 b2 = -b0;
    f64 a1 = -2.0 * cosw0;

    f64 a0 = 1.0 + alpha;
    f64 a2 = 1.0 - alpha;
   
    _biquad(quad_vals, channel, sample_count, a0, a1, a2, b0, b1, b2);
}

// seems simple enough
void naive_lowpass(f64* channel, u64 sample_count, f64 fraction) {
    //f64 prev = *channel;
    channel[0] *= fraction;
    channel++;
    for(u64 i = 1; i < sample_count; i++) {
        *channel = (*(channel-1) * (1.0-fraction) + *(channel) * (fraction));
        channel++;
    }
}


