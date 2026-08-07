#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "meter.h"
#include "wav.h"


// report peak and RMS for a whole file and per block (say, 100 ms blocks), plus a clipped-sample counter
void meter(char* filename) {
    FILE * file = fopen(filename , "rb");

    printf("opening file %s \n", filename);

    if (file == NULL)
        exit(1);

    printf("opened file\n");

    // obtain file size:
    fseek(file, 0, SEEK_END);
    u32 size = ftell(file);
    rewind(file);

    u8* buffer = (u8*)malloc(sizeof(u8) * size);
    if (buffer == NULL) 
        exit(2);

    // copy the file into the buffer:
    u32 result = fread(buffer, 1, size, file);
    if (result != size) 
        exit(3);


    FormatChunk* fc = (FormatChunk*)(buffer + sizeof(RiffChunk));

    u16 channel_count = fc->nbrChannels;
    u32 frequency = fc->frequency;
    u16 sample_bits = fc->bitsPerSample;

    DataChunk* dc = (DataChunk*)(buffer + sizeof(RiffChunk) + sizeof(FormatChunk));

    u32 data_bytes = dc->dataSize;
    u8* samples = dc->sampledData;

    // normalize to -1.0 - 1.0?

    // for now just handle the 2 channel 16 bit

    // per channel
    u32 sample_count = data_bytes / (sample_bits / 8) / channel_count;
    f64* left = malloc(sample_count * sizeof(f64));
    f64* right = malloc(sample_count * sizeof(f64));

    f64* l = left;
    f64* r = right;

    u32 clipped_sample_count = 0;
    u8 is_in_clip = 0;

    if (channel_count == 2 && sample_bits == 16) {
        // 
        u8* run = samples;

        for (u32 i = 0; i < sample_count; i++) {
            u8 ll = *(run + 0);
            u8 lh = *(run + 1);
            u8 rl = *(run + 2);
            u8 rh = *(run + 3);

            i16 left_raw = ((u16)lh << 8) + ll;
            *l = left_raw * 1.0 / 32768;

            i16 right_raw = ((u16)rh << 8) + rl;
            *r = right_raw * 1.0 / 32768;

            if (left_raw == 32767 || left_raw == -32768 || right_raw == 32767 || right_raw == -32768) {
                if (!is_in_clip)
                    clipped_sample_count++;
                is_in_clip = 1;
            } else {
                if (is_in_clip)
                    is_in_clip = 0;
            }

            //printf("%lf left, %lf right\n", *left, *right);

            l++;
            r++;

            run += 4;
        }
    }

    l = left;

    f64 peak = 0.0;
    f64 peak2 = 0.0;


    f64 sum_squared = 0.0;

    f64 block_peak = 0.0;
    f64 block_peak2 = 0.0;
    f64 block_sum_squared = 0.0;



    // let's say 100ms block
    // thus
    u32 block_sample_count = frequency * 100 / 1000;

    // just for entire file for now
    for (u32 i = 0; i < sample_count; i++) {

        if(i > 0 && (i % block_sample_count) == 0){
            f64 block_rms = pow(block_sum_squared/block_sample_count, .5);

            printf("Block peak was %lf, RMS was %lf\n", (block_peak < 0.0 ? -1.0 * block_peak: block_peak), block_rms);
            // new block, lets log at least
            block_peak = 0.0;
            block_peak2 = 0.0;
            block_sum_squared = 0.0;
        }

        // just look at L for now I guess

        f64 amp = *l;
        f64 amp2 = amp * amp;

        if (amp2 > peak2) {
            peak = amp;
            peak2 = amp2;
        }

        if (amp2 > block_peak2) {
            block_peak = amp;
            block_peak2 = amp2;
        }

        sum_squared += amp2;
        block_sum_squared += amp2;

        l++;
    }

    // sqrt ( 1/samplecoutn * (amp^2 + amp^2 .... ))

    f64 rms = pow(sum_squared/sample_count, .5);

    printf("File peak was %lf, RMS was %lf, clipped sample count %d\n", peak, rms, clipped_sample_count);

    // terminate
    fclose(file);
    free(buffer);
}

