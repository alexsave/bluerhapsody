#ifndef FORMAT_H
#define FORMAT_H

#include "types.h"
#include "wav.h"

static const u16 SAMPLE_BITS = 16; 
static const u32 SAMPLE_FREQUENCY = 48000;
static const u16 DURATION_S = 280;

static const u8 CHANNEL_COUNT = 2;

static const u32 NUM_SAMPLES = SAMPLE_FREQUENCY * DURATION_S;
static const u32 DATA_BYTES = (CHANNEL_COUNT * NUM_SAMPLES * SAMPLE_BITS) / 8;
static const u32 REAL_FILE_SIZE = sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk) + DATA_BYTES;

void* write_headers();

void write_samples(f64* left, f64* right, u8* out);

void write_wav(f64* left, f64* right, void* wav, char* filename);

#endif

