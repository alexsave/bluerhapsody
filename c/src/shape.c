#import <math.h>
#import "shape.h"
#import "types.h"

// given phase, give me triangle wave
// and phase here is baseically note * 2 * M_PI * j/ SAMPLE_FREQUENCY already

f64 sawtooth(f64 phase, u16 levels) {
    f64 sum = 0.0;
    for (u16 n = 1; n <= levels; n++) {
        sum -= sin(n * phase) / n;
    }
    return sum * 2.0 / M_PI;
}

f64 square(f64 phase, u16 levels) {
    f64 sum = 0.0;
    for (u16 n = 1; n <= levels; n+=2) {
        sum += sin(n * phase) / n;
    }
    return sum * 4.0 / M_PI;
}

f64 triangle(f64 phase, u16 levels) {
    f64 sum = 0.0;
    for (u16 n = 1; n <= levels; n+=2) {
        f64 unit = sin(n * phase) / n / n;
        if ((n&2) == 2) {
            sum -= unit;
        } else {
            sum += unit;
        }
    }
    return sum * 8.0 / M_PI / M_PI;
}

f64 sine(f64 phase) {
    return sin(phase);
}

