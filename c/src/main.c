#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "types.h"
#include "meter.h"
#include "wav.h"

static const u32 SAMPLE_FREQUENCY = 48000;
static const u16 SAMPLE_BITS = 16;
static const u16 DURATION_S = 15;

static const u8 CHANNEL_COUNT = 2;

static const u32 NUM_SAMPLES = SAMPLE_FREQUENCY * DURATION_S;
static const u32 DATA_BYTES = (CHANNEL_COUNT * NUM_SAMPLES * SAMPLE_BITS) / 8;
static const u32 REAL_FILE_SIZE = sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk) + DATA_BYTES;

// left will be mono in case of channel count == 1
// pass in f64s, let this turn it into whatever
void write_samples(f64* left, f64* right, u8* out) {
    // lets just look at that same spot as in the react ui
    for(f64 * r = left + (2 * SAMPLE_FREQUENCY); r < left + (2 * SAMPLE_FREQUENCY + 100); r++) {
        i16 sample_amplitude = (*r) * (1 << 15);
        printf("%hd\n", sample_amplitude);
        printf("%f\n", (*r)*(1<<15));
    }
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

void sawtooth(f64* l, f32 note, f32 effective_amplitude, u16 note_duration_ms ) {
    u16 levels = 20;
    // sawtooth
    for(u32 j = 0; j < SAMPLE_FREQUENCY / 1000 * note_duration_ms; j++) {
        for (u16 n = 1; n <= levels; n++) {
            *l += sin((n * note * 2 * M_PI * j) / SAMPLE_FREQUENCY) * -2.0 / M_PI / n;
        }
        
        //*l *= effective_amplitude;
        l++;
    }
}

void square(f64* l, f32 note, f32 effective_amplitude, u16 note_duration_ms) {
    u16 levels = 20;
    // square
    for(u32 j = 0; j < SAMPLE_FREQUENCY / 1000 * note_duration_ms; j++) {
        for (u16 n = 1; n <= levels; n++) {
            if((n & 1) == 0) continue;
            *l += sin((n * note * 2 * M_PI * j) / SAMPLE_FREQUENCY) * 4.0 / M_PI / n;
        }
        
        //*l *= effective_amplitude;
        l++;
    }
}

void triangle(f64* l, f32 note, f32 effective_amplitude, u16 note_duration_ms, u16 levels) { 
    // triangle
    for(u32 j = 0; j < SAMPLE_FREQUENCY / 1000 * note_duration_ms; j++) {
        for (u16 n = 1; n <= levels; n++) {
            if((n & 1) == 0) continue;
            
            // at 1 positive, at 3 negative //001 011
            if((n & 2) == 2) // subtract
                *l += sin((n * note * 2 * M_PI * j) / SAMPLE_FREQUENCY) * -8.0 / M_PI / M_PI / n / n;
            else // add
                *l += sin((n * note * 2 * M_PI * j) / SAMPLE_FREQUENCY) * 8.0 / M_PI / M_PI / n / n;
        }

        //*l *= effective_amplitude;
        l++;
    }
}


int main(int argc, char* argv[]){

    if (argc > 2) {
        if (strcmp(argv[1], "--spectrum") == 0){
            spectrum(argv[2]);
            return 0;
        }
        if (strcmp(argv[1], "--meter") == 0){
            meter(argv[2]);
            return 0;
        }
        if (strcmp(argv[1], "--diff") == 0){
            printf("deez\n");
            diff(argv[2], argv[3]);
            return 0;
        }
    }

    void* wav = write_headers();


    f32 gain = -12.0;

    // an increase of 10 DB means the POWER 10x
    // if you want to 10x the power, you need to sqrt(10)x the amplitude
    // 1 db increase is 10^1/10 increase in power, but sqrt(10)^1/10 increase in amplitude
    // or 10^1/20 increase in amplitude
    
    // power is correlated to square of amplitude, thus if you want to 10x
    const f64 AMP_MULT = pow(10.0, 1.0/20.0);
    printf("%lf\n", AMP_MULT);

    f64 effective_amplitude = pow(AMP_MULT, gain);

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

    u16 note_duration_ms = 5000;
    //u16 rest_duration_ms = 200;


    // create f64*

    f64* left = calloc(sizeof(f64), NUM_SAMPLES);
    //f64* right = calloc(sizeof(f64), NUM_SAMPLES);

    // just do A4 for now

    f64* l = left;
    //f64* r = right;

    // sin

    // 16 s in I hear something - roughly 32Hz

    // 100x, 1:24 loses me
    // my hearing is roughly 32Hz to 16.8kHz wow

    f32 note = 375.0;


    f32 base = note;
    f32 minor_third = base * SEMITONE_MULT * SEMITONE_MULT * SEMITONE_MULT;
    f32 fifth = base * 3.0 / 2.0;//SEMITONE_MULT * SEMITONE_MULT * SEMITONE_MULT * SEMITONE_MULT;
    f32 octave = base * 2.0;

    triangle(l, base, effective_amplitude, note_duration_ms, 8);
    l = l + SAMPLE_FREQUENCY;
    triangle(l, minor_third, effective_amplitude, note_duration_ms, 8);
    l = l + SAMPLE_FREQUENCY;
    triangle(l, fifth, effective_amplitude, note_duration_ms, 8);
    l = l + SAMPLE_FREQUENCY;
    triangle(l, octave, effective_amplitude, note_duration_ms, 8);


    // idk, but lets scale it such that the highest peak is at this amplitude

    f64 min = *left;
    f64 max = *left;
    f64 value = *left;
    for (f64 * r = left; r < left + NUM_SAMPLES; r++) {
        value = *r;
        if (value < min)
            min = value;
        else if (value > max)
            max = value;
    }
    printf("min %f max %f\n", min, max);
    
    if (min * -1.0 > max)
        max = -1.0 * min;
    
    
    f64 scaled_amplitude = effective_amplitude / max;
    printf("scaled amp %f\n", scaled_amplitude);

    // scale entire thing down
    for (l = left; l < left + NUM_SAMPLES; l++)
        *l *= scaled_amplitude;

    // lets just look at that same spot as in the react ui
    for(f64 * r = left + (2 * SAMPLE_FREQUENCY); r < left + (2 * SAMPLE_FREQUENCY + 100); r++) {
        printf("%f\n", *r);
        //*r *= scaled_amplitude;
        ////printf("%f\n", *r);
    }

    u8* sampled_data = (u8*)(wav + sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk));
    // DO NOT pass values not between -1 and 1 to this
    write_samples(left, left, sampled_data);


    // E, F, F#, G, G#, A, A#, B, C, C#, D, D#

    FILE * file = fopen("m02_additive_chord.wav" , "wb");

    if (!file)
        return 1;
    fwrite((const void *)wav,  sizeof(u8), REAL_FILE_SIZE, file);

    fclose(file);


    free(wav);
    free(left);

    return 0;
}

