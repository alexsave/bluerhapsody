#ifndef METER_H
#define METER_H

#include "types.h"

// all we really need when reading
typedef struct WavMetadata {
    u32 sample_count;
    u32 frequency;
} WavMetadata;

// just print out for now
void meter(char* filename);
void diff(char* filename1, char* filename);
void spectrum(char* filename);

#endif

