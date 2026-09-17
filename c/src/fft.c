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

    // Hz = bin# * frequency / bin_count;

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
        printf("angle %lf complex %lf+%lfi\n", angle, rots[i].re, rots[i].im);
    }

    exit(1);


    //f64 amp = left_ptr[s];

    u32 stride = 1;

    while (stride < bin_count) {

        u32 run = 0;
        while (run < bin_count) {

            for (u32 substep = 0; substep < stride; substep++) {

                //Cpx rot = ?
                /*
                    stride 1, substep 0
                    (e ^  (i * pi / 1) ) ^ 0

                    stride 2, 
                        substep 0 and 
                            (e ^  (i * pi / 2)) ^ 0
                        substep 1
                            (e ^  (i * pi / 2)) ^ 1
                    stride 4
                        substep 1
                            (e ^  (i * pi / 4)) ^ 1

                    cool but what is angle
                    45 = f( 4, 1)
                    PI / 4 * 1
                    angle = substep * PI / stride
                    
                    
                */

                f64 angle = substep * M_PI / stride;
                printf("angle %d\n", angle);

                Cpx rot = { .re = cos(angle), .im = sin(angle)};

                /*
                lets say we have all 2^20 rotations predefined
                stride 4 substep 1
                itll be at index 2^20/4 * 1

                but what will be max stride? actually 2^19
                */



                Cpx a = array[run];
                Cpx b = array[run];

                //array[run].re = a.re + 
                //array[run].im = a.im + 

                //array[run + stride].re = a.re -
                //array[run + stride].im = a.im -
                
                run++;
            }

            // skip over to the next sub-FFT
            run += stride;
        }
        // double stride
        stride <<= 1;
    }


}

