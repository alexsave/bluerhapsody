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


Cpx* fft(f64* channel, u32 sample_count, u64* bin_count) {

    u8 log_bin_count = 0;

    //printf("bin count %d, log is %d, sample_count\n", *bin_count, log_bin_count, sample_count);

    if (*bin_count < sample_count) {
        *bin_count = 1;
        while (*bin_count < sample_count) {
            *bin_count = *bin_count * 2;
            log_bin_count++;
        } 
    }



    // this alternates real and imaginary for cache lines
    Cpx* array = calloc(*bin_count, sizeof(Cpx));
    //printf("done allocating complex array\n");

    for (u32 i = 0; i < sample_count; i++) {
        array[i].re = channel[i];
    }
    //printf("done filling complex array\n");


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
            // will this work with struct? idk;
            temp = array[i];
            array[i] = array[inverse];
            array[inverse] = temp;
        }
    }


    //printf("creating nudges\n");
    // pre calcualte rotations
    // max stride will be bin_count/2
    // this COULD be lazy init
    Cpx* rots = calloc(*bin_count >> 1, sizeof(Cpx));

    for (u32 i = 0; i < (*bin_count >> 1); i++) {
        f64 angle = i * M_PI / (*bin_count >> 1);

        rots[i].re = cos(angle);
        rots[i].im = sin(angle);
        //printf("angle %lf complex %lf+%lfi\n", angle, rots[i].re, rots[i].im);
    }
    //printf("done creating nudges\n");


    //f64 amp = left_ptr[s];

    u32 stride = 1;

    while (stride < *bin_count) {

        u32 rotation_index_step = (*bin_count >> 1) / stride;

        u32 run = 0;
        while (run < *bin_count) {

            for (u32 substep = 0; substep < stride; substep++) {

                // use precomputed
                Cpx rot = rots[rotation_index_step * substep];

                Cpx b = array[run + stride];
                // (a+bi)(c+di)

                // a * c + a * d * i + b * i * c + d * i * b * i
                // (a * c - b * d) +  (a * d + b * c) * i
                Cpx rot_b = {.re = b.re*rot.re - b.im*rot.im , .im = b.im*rot.re + b.re*rot.im};

                Cpx a = array[run];



                array[run].re = a.re + rot_b.re;
                array[run].im = a.im + rot_b.im;

                array[run + stride].re = a.re - rot_b.re;
                array[run + stride].im = a.im - rot_b.im;

                //if (run == 0) {
                //printf("slot 0 %lf %lf\n", array[run].re, array[run].im);
                //printf("a %lf %lf rot b %lf %lf\n", a.re, a.im, rot_b.re, rot_b.im);
                //}

                run++;
            }

            // skip over to the next sub-FFT
            run += stride;
        }
        // double stride
        stride <<= 1;
    }

    // Hz = bin# * frequency / bin_count;

    //printf("freeing rots\n");
    free(rots);
    //printf("freed rots\n");


    //wm.frequency
    //printf("%lf %lf\n", ((f64)((u64)b * wm.frequency) / bin_count), pow(ft_r[b] * ft_r[b] + ft_i[b] * ft_i[b], 0.5));

    return array;
}

// sample count + channel describe the input, bin_count is requested
// return window coutn
u64 fft_windows(f64* channel, u32 sample_count, u64* bin_count, Cpx** out_cpx) {

    if (*bin_count == 0) {
        Cpx* cpx = fft(channel, sample_count, bin_count);
        *out_cpx = cpx;
        return 1;
    }

    // if you have 48000, and you request bin_count 8192

    // lets just say we do something like window 0 , 0-8192, window 1, 4096-12k, window 2, 8192-16k, 12k-20k, 16k-24k
    // if you have 4000 samples, 1 window
    // if you have 4096 samples, 2 windows
    // if you have  8000 samples, 2 windows

    // 10000 samples, 8192 window size, 4096 incrmenet
    // 0. 0-8192
    // 1. 4096-12000 
    // 4096 + 8192 > 10000
    // 10000 -4096

    // 2. 8192 

    // round up to nearest bin_count/2, that's number of windwos you'll have

    u32 window_size = *bin_count;
    u32 increment = window_size / 2;

    //8191 - 2 windows
    //8192 - 2 windows
    //8193 - 3 windows

    u64 window_count = sample_count / increment;
    if (sample_count % increment != 0)
        window_count++;

    (*out_cpx) = calloc(window_count * window_size, sizeof(Cpx));

    //printf("memallocated\n");

    Cpx * out_run = *out_cpx;

    //sample_count / increment;

    for (u32 i = 0; i < window_count; i++){
        //printf("working through window #%d\n", i);
        u32 effective_sample_count = window_size;
        if (i * increment + window_size > sample_count){
            effective_sample_count = sample_count - i * increment;
        }
        // do it, but break
        //printf("calling fft\n");
        Cpx * cpx = fft(channel + i * increment, effective_sample_count, bin_count);
        //printf("done calling fft\n");

        //printf("about to copy mem \n");
        memcpy(out_run, cpx, window_size);
        //printf("about to free cpx\n");
        free(cpx);
        //printf("freed cpx\n");

        out_run += *bin_count;
    }

    return window_count;
}

Cpx* fast(char* filename) {
    f64* right_ptr = 0;
    f64* left_ptr = 0;
    WavMetadata wm = get_channels(filename, &left_ptr, &right_ptr);

    // so how do we want to do this?
    // first off let's see if we can get it working for a single DFT over the entire file
    // how many bins to choose?
    // I guess make it some 2^n value
    // this will be clear why when we do FFT 
    u64 bin_count;

    Cpx* array = fft(left_ptr, wm.sample_count, &bin_count);
    for (u32 i = 0; i < bin_count; i++) {
        // print out magnitude? (a+bi) => (a*a + b*b)
        printf("%lfHz: %lf\n", ((f64)i * (f64)wm.frequency) / (f64)bin_count, array[i].re * array[i].re + array[i].im * array[i].im);
    }
    return array;
}

