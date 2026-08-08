#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "meter.h"
#include "wav.h"


// first off lets abstract something like filename -> f64*

// returns sample count
// might need to return frequency or something later
WavMetadata get_channels(char* filename, f64** right_ptr, f64** left_ptr) {
    FILE * file = fopen(filename, "rb");

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
    f64* right = 0;

    if (channel_count == 2)
        right = malloc(sample_count * sizeof(f64));

    f64* l = left;
    f64* r = right;

    //u32 clipped_sample_count = 0;
    //u8 is_in_clip = 0;
        u8* run = samples;

    if (channel_count == 2 && sample_bits == 16) {
        for (u32 i = 0; i < sample_count; i++) {
            u8 ll = *(run + 0);
            u8 lh = *(run + 1);
            u8 rl = *(run + 2);
            u8 rh = *(run + 3);

            i16 left_raw = ((u16)lh << 8) + ll;
            *l = left_raw * 1.0 / 32768;

            i16 right_raw = ((u16)rh << 8) + rl;
            *r = right_raw * 1.0 / 32768;

            /*if (left_raw == 32767 || left_raw == -32768 || right_raw == 32767 || right_raw == -32768) {
                if (!is_in_clip)
                    clipped_sample_count++;
                is_in_clip = 1;
            } else {
                if (is_in_clip)
                    is_in_clip = 0;
            }*/

            //printf("%lf left, %lf right\n", *left, *right);

            l++;
            r++;

            run += 4;
        }
    } else if (channel_count == 1 && sample_bits == 16) {
        for (u32 i = 0; i < sample_count; i++) {
            u8 ll = *(run + 0);
            u8 lh = *(run + 1);

            i16 left_raw = ((u16)lh << 8) + ll;
            *l = left_raw * 1.0 / 32768;

            /*if (left_raw == 32767 || left_raw == -32768 || right_raw == 32767 || right_raw == -32768) {
                if (!is_in_clip)
                    clipped_sample_count++;
                is_in_clip = 1;
            } else {
                if (is_in_clip)
                    is_in_clip = 0;
            }*/

            //printf("%lf left, %lf right\n", *left, *right);

            l++;

            run += 2;
        }
    } else if (channel_count == 2 && sample_bits == 8) {
        for (u32 i = 0; i < sample_count; i++) {
            u8 lb = *(run + 0);
            u8 rb = *(run + 1);

            *l = (1.0 * lb - 128.0) / 128.0;
            *r = (1.0 * rb- 128.0) / 128.0;

            /*if (left_raw == 255 || left_raw == 0 || right_raw == 255 || right_raw == 0) {
                if (!is_in_clip)
                    clipped_sample_count++;
                is_in_clip = 1;
            } else {
                if (is_in_clip)
                    is_in_clip = 0;
            }*/

            l++;
            r++;

            run += 2;
        }
    } else if (channel_count == 1 && sample_bits == 8) {
        for (u32 i = 0; i < sample_count; i++) {
            u8 lb = *(run + 0);

            *l = (1.0 * lb - 128.0) / 128.0;

            /*if (left_raw == 255 || left_raw == 0) {
                if (!is_in_clip)
                    clipped_sample_count++;
                is_in_clip = 1;
            } else {
                if (is_in_clip)
                    is_in_clip = 0;
            }*/

            l++;

            run++;
        }
    }

    *right_ptr = right;
    *left_ptr = left;

    // terminate
    fclose(file);
    free(buffer);

    WavMetadata wm = {.sample_count = sample_count, .frequency = frequency};
    return wm;
}


void volume_stats(f64* stream, u32 sample_count, u32 frequency) {
    f64* l = stream;

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

    //printf("File peak was %lf, RMS was %lf, clipped sample count %d\n", peak, rms, clipped_sample_count);
    printf("File peak was %lf, RMS was %lf\n", peak, rms);
    

}

// report peak and RMS for a whole file and per block (say, 100 ms blocks), plus a clipped-sample counter
void meter(char* filename) {
    f64* right_ptr = 0;
    f64* left_ptr = 0;
    WavMetadata wm = get_channels(filename, &left_ptr, &right_ptr);
    volume_stats(left_ptr, wm.sample_count, wm.frequency);

    free(left_ptr);
    if(right_ptr != 0)
        free(right_ptr);
}

void diff(char* filename1, char* filename2) {
    printf("diff\n");
    f64* file1_left = 0;
    f64* file1_right = 0;
    WavMetadata wm1 = get_channels(filename1, &file1_left, &file1_right);
    u32 file1_sample_count = wm1.sample_count;
    printf("diff\n");

    f64* file2_left = 0;
    f64* file2_right = 0;
    WavMetadata wm2 = get_channels(filename2, &file2_left, &file2_right);
    u32 file2_sample_count = wm2.sample_count;
    printf("diff\n");

    // only look at part that matters
    // if sample count is different obviously the audio is not the same
    u32 min_sample_count = file1_sample_count > file2_sample_count ? file2_sample_count : file1_sample_count;
    printf("diff\n");

    f64* diff = malloc(sizeof(f64) * min_sample_count);
    for (u32 i = 0; i < min_sample_count; i++){
        // just left for now
        // we basicaly want rms and peak of the diff

        diff[i] = file1_left[i] - file2_left[i];
    }

    // run this through peak and rms
    printf("diff created %d samples\n", min_sample_count);

    // they could be different frequency idk
    volume_stats(diff, min_sample_count, wm1.frequency);

    free(diff);
    free(file1_left);
    free(file2_left);

    if(file1_right != 0)
        free(file1_right);

    if(file2_right != 0)
        free(file2_right);

}
