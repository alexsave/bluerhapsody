#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "meter.h"

// we cant do 1 -> 1000000000000000000000000000000
u32 bit_inverse(u32 index, u8 size) {
    // including early break if resulting inverse is > index

    u32 inverse = 0;
    for (u8 bit = 0; bit < size; bit++)
        inverse |= ( ((index >> bit) & 1) << (size - 1 - bit));

    return inverse;
}

void fast(char* filename) {
    f64* right_ptr = 0;
    f64* left_ptr = 0;
    WavMetadata wm = get_channels(filename, &left_ptr, &right_ptr);

    // so how do we want to do this?
    // first off let's see if we can get it working for a single DFT over the entire file
    // how many bins to choose?
    // I guess make it some 2^n value
    // this will be clear why when we do FFT 

    u32 sample_count = wm.sample_count;

    u8 log_bin_count = 0;

    u32 bin_count = 1;
    while (bin_count < sample_count) {
        bin_count <<= 1;
        log_bin_count++;
    } 
    printf("bin count %d, log is %d\n", bin_count, log_bin_count);


    f64* array = calloc(bin_count, sizeof(f64));

    printf("before memcopy\n");
    //memcpy((void*)array, (void*)left_ptr, bin_count * sizeof(f64));
    memcpy(array, left_ptr, bin_count);
    printf("after memcopy\n");

    // Hz = bin# * frequency / bin_count;

//2^20 bins, 1 # 48000
    //48000/2^20

    // we're going to operate on left_ptr, entirely in place btw


    u32 inverse = 0;
    f64 temp = 0;
    // BIT INVERSE ORDER
    for (u32 i = 0; i < sample_count; i++) {
        inverse = bit_inverse(i, log_bin_count);
        //printf("i %d and inverse %d\n", i, inverse);
        if (inverse > i) {
            //swap
            temp = array[i];
            array[i] = array[inverse];
            array[inverse] = temp;
        }
    }

    //f64 amp = left_ptr[s];
}

