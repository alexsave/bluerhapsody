#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "meter.h"
#include "fft.h"

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


    // this alternates real and imaginary for cache lines
    Cpx* array = calloc(bin_count, sizeof(Cpx));

    for (u32 i = 0; i < sample_count; i++) {
        array[i].re = left_ptr[i];
    }


//2^20 bins, 1 # 48000
    //48000/2^20

    // we're going to operate on left_ptr, entirely in place btw


    u32 inverse = 0;
    Cpx temp;
    // BIT INVERSE ORDER
    for (u32 i = 0; i < sample_count; i++) {
        inverse = bit_inverse(i, log_bin_count);
        //printf("i %d and inverse %d\n", i, inverse);
        if (inverse > i) {
            //swap
            // will this work with struct? idk
            temp = array[i];
            array[i] = array[inverse];
            array[inverse] = temp;
        }
    }


    // pre calcualte rotations
    // max stride will be bin_count/2
    // this COULD be lazy init
    Cpx* rots = calloc(bin_count >> 1, sizeof(Cpx));

    for (u32 i = 0; i < (bin_count >> 1); i++) {
        f64 angle = i * M_PI / (bin_count >> 1);

        rots[i].re = cos(angle);
        rots[i].im = sin(angle);
        //printf("angle %lf complex %lf+%lfi\n", angle, rots[i].re, rots[i].im);
    }


    //f64 amp = left_ptr[s];

    u32 stride = 1;

    while (stride < bin_count) {

        u32 rotation_index_step = (bin_count >> 1) / stride;

        u32 run = 0;
        while (run < bin_count) {

            for (u32 substep = 0; substep < stride; substep++) {

                // use precomputed
                Cpx rot = rots[rotation_index_step * substep];

                Cpx b = array[run + stride];
                // (a+bi)(c+di)

                // a * c + a * d * i + b * i * c + d * i * b * i
                // (a * c - b * d) +  (a * d + b * c) * i
                Cpx rot_b = {.re = b.re*rot.re - b.im*rot.im , .im = b.im*rot.re + b.re*rot.im};

                Cpx a = array[run];
            
                array[run].re = a.re + b.re;
                array[run].im = a.im + b.im;
                
                array[run + stride].re = a.re - b.re;
                array[run + stride].im = a.im - b.im;
            
                run++;
            }

            // skip over to the next sub-FFT
            run += stride;
        }
        // double stride
        stride <<= 1;
    }

    // Hz = bin# * frequency / bin_count;

    for (u32 i = 0; i < bin_count; i++) {
        // print out magnitude? (a+bi) => (a*a + b*b)
        //printf("%lfHz: %lf\n", (f64) wm.frequency) , array[i].re * array[i].re + array[i].im * array[i].im);
        printf("%lfHz: %lf\n", ((f64)i * (f64)wm.frequency) / (f64)bin_count, array[i].re * array[i].re + array[i].im * array[i].im);
    }
    
    //wm.frequency
    //printf("%lf %lf\n", ((f64)((u64)b * wm.frequency) / bin_count), pow(ft_r[b] * ft_r[b] + ft_i[b] * ft_i[b], 0.5));


}

