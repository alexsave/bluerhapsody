#include <stdio.h>
#include <stdlib.h>

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

    DataChunk* dc = (FormatChunk*)(buffer + sizeof(RiffChunk) + sizeof(FormatChunk));

    u32 data_bytes = dc->dataSize;
    u8* samples = dc->sampledData;

    // normalize to -1.0 - 1.0?

    // for now just handle the 2 channel 16 bit

    // per channel
    u32 sample_count = data_bytes / (sample_bits / 8) / channel_count;
    f64* left = malloc(sample_count * sizeof(f64));
    f64* right = malloc(sample_count * sizeof(f64));

    if (channel_count == 2 && sample_bits == 16) {
        // 
        u8* run = samples;

        for (u32 i = 0; i < sample_count; i++) {

            u8 ll = *(run + 0);
            u8 lh = *(run + 1);
            u8 rl = *(run + 2);
            u8 rh = *(run + 3);

            i16 left_raw = ((u16)lh << 8) + ll;
            *left = left_raw * 1.0 / 32768;

            i16 right_raw = ((u16)rh << 8) + rl;
            *right = right_raw * 1.0 / 32768;


            printf("%lf left, %lf right\n", *left, *right);

            left++;
            right++;

            run += 4;
        }
    }






    // terminate
    fclose(file);
    free(buffer);
}

