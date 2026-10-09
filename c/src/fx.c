#include "fx.h"
#include "types.h"

// seems simple enough
void lowpass(f64* channel, u64 sample_count, f64 fraction) {
    //f64 prev = *channel;
    channel[0] *= fraction;
    channel++;
    for(u64 i = 1; i < sample_count; i++) {
        *channel = (*(channel-1) * (1.0-fraction) + *(channel) * (fraction));
        channel++;
    }
}


