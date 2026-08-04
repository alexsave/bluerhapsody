#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "types.h"

typedef struct RiffChunk {
    u32 fileTypeBlocID;//  RIFF  (0x52, 0x49, 0x46, 0x46)
    u32 fileSize;//        (4 bytes) : Overall file size minus 8 bytes
    u32 fileFormatID;//    WAVE   (0x57, 0x41, 0x56, 0x45)
} RiffChunk;
 
typedef struct FormatChunk {
    u32 formatBlocID;//    (4 bytes) : Identifier « fmt␣ »  (0x66, 0x6D, 0x74, 0x20)
    u32 blocSize;//        (4 bytes) : Chunk size minus 8 bytes, which is 16 bytes here  (0x10)
    u16 audioFormat;//     (2 bytes) : Audio format (1: PCM integer, 3: IEEE 754 float)
    u16 nbrChannels;//     (2 bytes) : Number of channels
    u32 frequency;//       (4 bytes) : Sample rate (in hertz)
    u32 bytePerSec;//      (4 bytes) : Number of bytes to read per second (Frequency * BytePerBloc).
    u16 bytePerBloc;//     (2 bytes) : Number of bytes per block (NbrChannels * BitsPerSample / 8).
    u16 bitsPerSample;//   (2 bytes) : Number of bits per sample
} FormatChunk;

typedef struct DataChunk {
    u32 dataBlocID;//      (4 bytes) : Identifier « data »  (0x64, 0x61, 0x74, 0x61)
    u32 dataSize;//        (4 bytes) : SampledData size
    u8 sampledData[];
} DataChunk;
 
static const u32 SAMPLE_FREQUENCY = 48000;
static const u16 SAMPLE_BITS = 8;
static const u16 DURATION_S = 10;

static const u32 NUM_SAMPLES = SAMPLE_FREQUENCY * DURATION_S;
static const u32 DATA_BYTES = (NUM_SAMPLES * SAMPLE_BITS) / 8;
static const u32 REAL_FILE_SIZE = sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk) + DATA_BYTES;

int main(int argc, char* argv[]){

    void* wav = malloc(REAL_FILE_SIZE);
    
    RiffChunk * rf = (RiffChunk*)wav;
    rf->fileTypeBlocID = (((((0x46 << 8) + 0x46) << 8) + 0x49) << 8) + 0x52;
    

    rf->fileFormatID = (((((0x45 << 8) + 0x56) << 8) + 0x41) << 8) + 0x57;

    FormatChunk * fc = (FormatChunk*)(wav + sizeof(RiffChunk));
    fc->formatBlocID = (((((0x20 << 8) + 0x74) << 8) + 0x6D) << 8) + 0x66;
    fc->blocSize = 16;

    fc->audioFormat = 1;
    fc->nbrChannels = 1;
    fc->frequency = SAMPLE_FREQUENCY;
    fc->bitsPerSample = SAMPLE_BITS;

    fc->bytePerBloc = (fc->nbrChannels * SAMPLE_BITS / 8);
    fc->bytePerSec = SAMPLE_FREQUENCY * fc->bytePerBloc;

    DataChunk * dc = (DataChunk*)(wav + sizeof(RiffChunk) + sizeof(FormatChunk));
    dc->dataBlocID = (((((0x61 << 8) + 0x74) << 8) + 0x61) << 8) + 0x64;

    dc->dataSize = DATA_BYTES;

    // only FileFormatID from RiffChunk is used here
    // "Overall file size minus 8 bytes"
    rf->fileSize = REAL_FILE_SIZE - 8;
    
    u8* sampled_data = (u8*)(wav + sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk));

    // parameters
    u16 note_frequency = 440;

    f32 amplitude = .05;

    // frequency means that it does 440 full rotations through circle in 1 second
    // thus y = sin(2pi*x)
    // thus y = sin(440 * 2pi*x)
    // if we step through sample by sample, x will be i/SAMPLE_FREQ

    // i will go all the way up to SAMPLE_FREQ * DURATION
    
    // now we can do fun stuff with sampled data


    for (u32 i = 0; i < NUM_SAMPLES; i++) {

        if (SAMPLE_BITS == 16) {
            i16 sample_amplitude = (1<<15) * amplitude * sin((note_frequency * 2 * M_PI * i) / SAMPLE_FREQUENCY);

            printf("%d %d\n", i, sample_amplitude);

            // hardcoded specific to 16-bit samples
            sampled_data[2 * i] = sample_amplitude & 255;
            sampled_data[2 * i  + 1] = sample_amplitude >> 8;
        } else if (SAMPLE_BITS == 8) {
            u8 sample_amplitude = 128 + (128 * amplitude * sin((note_frequency * 2 * M_PI * i) / SAMPLE_FREQUENCY));
            printf("%d %d %f\n", i, sample_amplitude, sin((note_frequency * 2 * M_PI * i)/SAMPLE_FREQUENCY));
            sampled_data[i] = sample_amplitude;
        }

    }

    FILE * file;
    file = fopen("1b.wav" , "wb");

    if (!file)
        return 1;

    fwrite((const void *)wav,  sizeof(u8), REAL_FILE_SIZE, file);

    fclose(file);
    free(wav);

    return 0;
}

