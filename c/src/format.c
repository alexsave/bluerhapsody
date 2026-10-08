#include <stdlib.h>
#include <stdio.h>

#include "format.h"
#include "wav.h"
#include "types.h"

// left will be mono in case of channel count == 1
// pass in f64s, let this turn it into whatever
void write_samples(f64* left, f64* right, u8* out, u16 seconds) {

    u32 num_samples = SAMPLE_FREQUENCY * seconds;
    

    // lets just look at that same spot as in the react ui
    if (SAMPLE_BITS == 16) {
        for (u32 i = 0; i < num_samples; i++) {
            i16 sample_amplitude = (*left) * (1 << 15);
            *out = sample_amplitude & 255;
            *(out + 1) = sample_amplitude >> 8;

            if (CHANNEL_COUNT == 2) {
                out += 2;
                sample_amplitude = (*right) * (1 << 15);
                *out = sample_amplitude & 255;
                *(out + 1) = sample_amplitude >> 8;
                right++;
        
            }   

            out += 2;
            left++;

        }   
    } else if (SAMPLE_BITS == 8) {
        for (u32 i = 0; i < num_samples; i++) {
            u8 sample_amplitude = ((*left)+ 1.0)*128;
            *out = sample_amplitude;
            if (CHANNEL_COUNT == 2) {
                out++;
                sample_amplitude = ((*right)+ 1.0)*128;
                *out = sample_amplitude;
                right++;
            }   

            out++;
            left++;
        }   
    }   
}

// returns wav ready to go
void* write_headers(u16 seconds) {
    u32 num_samples = SAMPLE_FREQUENCY * seconds;
    u32 data_bytes = (CHANNEL_COUNT * num_samples * SAMPLE_BITS) / 8;
    u32 real_file_size = sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk) + data_bytes;

    void* wav = malloc(real_file_size);

    RiffChunk * rf = (RiffChunk*)wav;
    rf->fileTypeBlocID = (((((0x46 << 8) + 0x46) << 8) + 0x49) << 8) + 0x52;


    rf->fileFormatID = (((((0x45 << 8) + 0x56) << 8) + 0x41) << 8) + 0x57;

    FormatChunk * fc = (FormatChunk*)(wav + sizeof(RiffChunk));
    fc->formatBlocID = (((((0x20 << 8) + 0x74) << 8) + 0x6D) << 8) + 0x66;
    fc->blocSize = 16;

    fc->audioFormat = 1;
    fc->nbrChannels = CHANNEL_COUNT;
    fc->frequency = SAMPLE_FREQUENCY;
    fc->bitsPerSample = SAMPLE_BITS;

    fc->bytePerBloc = (fc->nbrChannels * SAMPLE_BITS / 8);
    fc->bytePerSec = SAMPLE_FREQUENCY * fc->bytePerBloc;

    DataChunk * dc = (DataChunk*)(wav + sizeof(RiffChunk) + sizeof(FormatChunk));
    dc->dataBlocID = (((((0x61 << 8) + 0x74) << 8) + 0x61) << 8) + 0x64;

    dc->dataSize = data_bytes;

    // only FileFormatID from RiffChunk is used here
    // "Overall file size minus 8 bytes"
    rf->fileSize = real_file_size - 8;

    return wav;
}

void write_wav(f64* left, f64* right, void* wav, char* filename, u16 seconds) {
    u8* sampled_data = (u8*)(wav + sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk));

    RiffChunk * rf = (RiffChunk*)wav;
    u32 real_file_size = rf->fileSize + 8;


    // DO NOT pass values not between -1 and 1 to this
    write_samples(left, right, sampled_data, seconds);


    // E, F, F#, G, G#, A, A#, B, C, C#, D, D#

    FILE * file = fopen(filename , "wb");

    if (!file){
        exit(1);
    }
    fwrite((const void *)wav,  sizeof(u8), real_file_size, file);

    fclose(file);
}

