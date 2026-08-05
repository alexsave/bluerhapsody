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
static const u16 SAMPLE_BITS = 16;
static const u16 DURATION_S = 70;

static const u8 CHANNEL_COUNT = 2;

static const u32 NUM_SAMPLES = SAMPLE_FREQUENCY * DURATION_S;
static const u32 DATA_BYTES = (CHANNEL_COUNT * NUM_SAMPLES * SAMPLE_BITS) / 8;
static const u32 REAL_FILE_SIZE = sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk) + DATA_BYTES;

// left will be mono in case of channel count == 1
// pass in f64s, let this turn it into whatever
void write_samples(f64* left, f64* right, u8* out) {
    if (SAMPLE_BITS == 16) {
        for (u32 i = 0; i < NUM_SAMPLES; i++) {
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
        for (u32 i = 0; i < NUM_SAMPLES; i++) {
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
void* write_headers() {
    void* wav = malloc(REAL_FILE_SIZE);

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

    dc->dataSize = DATA_BYTES;

    // only FileFormatID from RiffChunk is used here
    // "Overall file size minus 8 bytes"
    rf->fileSize = REAL_FILE_SIZE - 8;

    return wav;
}

int main(int argc, char* argv[]){

    void* wav = write_headers();


    f32 gain = -15.0;

    const f64 DB_MULT = pow(10.0, 1.0/10.0);
    printf("%lf\n", DB_MULT);

    f32 effective_amplitude = pow(DB_MULT, gain);

    // frequency means that it does 440 full rotations through circle in 1 second
    // thus y = sin(2pi*x)
    // thus y = sin(440 * 2pi*x)
    // if we step through sample by sample, x will be i/SAMPLE_FREQ

    // i will go all the way up to SAMPLE_FREQ * DURATION

    // now we can do fun stuff with sampled data



    const f64 SEMITONE_MULT = pow(2.0, 1.0/12.0);

    // BEHOLD - the ladder of semitones
    /*static const u16 A3 = A4 >> 1;
    static const u16 A4 = 440;
    static const u16 A5 = A4 << 1;
    const u16 AS3 = A3 * SEMITONE_MULT;
    const u16 B3 = A3 * SEMITONE_MULT * SEMITONE_MULT;
    const u16 C3 = A3 * SEMITONE_MULT * SEMITONE_MULT* SEMITONE_MULT;
    const u16 CS3 = A3 * SEMITONE_MULT * SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT;;
    const u16 D3 = A3 * SEMITONE_MULT * SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT;
    const u16 DS3 = A3 * SEMITONE_MULT * SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT;
    const u16 E3 = A3 * SEMITONE_MULT * SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT;
    const u16 F3 = A3 * SEMITONE_MULT * SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT;
    const u16 FS3 = A3 * SEMITONE_MULT * SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT;
    const u16 G3 = A3 * SEMITONE_MULT * SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT;
    const u16 GS3 = A3 * SEMITONE_MULT * SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT* SEMITONE_MULT;
        b*/

    u16 note_duration_ms = 10000;
    u16 rest_duration_ms = 200;


    // create f64*

    f64* left = calloc(sizeof(f64), NUM_SAMPLES);
    f64* right = calloc(sizeof(f64), NUM_SAMPLES);

    f32 note = 110.0;

    f64* l = left;
    f64* r = right;
    for (u8 i = 0; i < 6; i++) {
        for(u32 j = 0; j < SAMPLE_FREQUENCY / 1000 * note_duration_ms; j++) {
            //*l = ((NUM_SAMPLES - (l- left)))*effective_amplitude * sin((note * 2 * M_PI * j) / SAMPLE_FREQUENCY)/ NUM_SAMPLES;

            f32 base = note * 2.0;
            f32 fifth = note * 3.0;
            
            // note that sin(a) + sin(b) = 2 * sin((a+b)/2) * cos((a-b)/2)

            *l = sin((base * 2 * M_PI * j) / SAMPLE_FREQUENCY);
            *l += sin((fifth * 2 * M_PI * j) / SAMPLE_FREQUENCY);
            *l *= effective_amplitude;
            //printf("%lf\n", *l);
            l++;

            

            //*r = ((r-right))*effective_amplitude * sin((note * 2 * M_PI * j) / SAMPLE_FREQUENCY) / NUM_SAMPLES;
            //r++;
        }
        l += SAMPLE_FREQUENCY * rest_duration_ms / 1000;
        r += SAMPLE_FREQUENCY * rest_duration_ms / 1000;
        note *= SEMITONE_MULT;
    }


    u8* sampled_data = (u8*)(wav + sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk));
    write_samples(left, left, sampled_data);


    // E, F, F#, G, G%, A, A#, B, C, C#, D, D#

    FILE * file = fopen("2a.wav" , "wb");

    if (!file)
        return 1;
    fwrite((const void *)wav,  sizeof(u8), REAL_FILE_SIZE, file);

    fclose(file);


    free(wav);
    free(left);

    return 0;
}

