#ifndef METER_H
#define METER_H

#include "types.h"

// all we really need when reading
typedef struct WavMetadata {
    u32 sample_count;
    u32 frequency;
} WavMetadata;

//maybe???
typedef struct WasmChannels {
    u32 sample_count;
    u32 frequency;
    f64* left_channel;
    f64* right_channel;
} WasmChannels;

// just print out for now
void meter(char* filename);
void diff(char* filename1, char* filename);
void spectrum(char* filename);
WavMetadata get_channels(char* filename, f64** right_ptr, f64** left_ptr);

#endif

