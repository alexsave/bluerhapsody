#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "types.h"
#include "parser.h"
#include "constants.h"
#include "note.h"
#include "meter.h"
#include "wav.h"
#include "voice.h"
#include "shape.h"

#include "fft.h"
#include "pq.h"

static const u32 SAMPLE_FREQUENCY = 48000;
static const u16 SAMPLE_BITS = 16;
static const u16 DURATION_S = 280;

static const u8 CHANNEL_COUNT = 2;

static const u32 NUM_SAMPLES = SAMPLE_FREQUENCY * DURATION_S;
static const u32 DATA_BYTES = (CHANNEL_COUNT * NUM_SAMPLES * SAMPLE_BITS) / 8;
static const u32 REAL_FILE_SIZE = sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk) + DATA_BYTES;

// left will be mono in case of channel count == 1
// pass in f64s, let this turn it into whatever
void write_samples(f64* left, f64* right, u8* out) {
    // lets just look at that same spot as in the react ui
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

    if (argc > 2) {
        if (strcmp(argv[1], "--fft") == 0){
            fast(argv[2]);
            return 0;
        }

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

    sf_reserve();

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


    u16 note_duration_ms = 5000;

    f64* left = calloc(sizeof(f64), NUM_SAMPLES);
    f64* right = calloc(sizeof(f64), NUM_SAMPLES);

    // just do A4 for now

    f64* l = left;
    f64* r = right;


    // 16 s in I hear something - roughly 32Hz

    // 100x, 1:24 loses me
    // my hearing is roughly 32Hz to 16.8kHz wow

    u16 PIANO_KEYS = 88;

    Voice* piano = calloc(PIANO_KEYS, sizeof(Voice));
    Voice* temp;

    f64 note = 440.0;

    f64 octave = 27.5;

    // these are indexes on piano lol

    for (u16 i = 0; i < PIANO_KEYS; i++) {
        if (i%12 == 0) {
            // it's an A, reset
            note = octave;
            octave *= 2.0;
        }

        temp = voice_init(SAMPLE_FREQUENCY, TYPE_TRIANGLE, note, 50, 50, .5, 500);
        memcpy(piano + i, temp, sizeof(Voice));
        note *= SEMITONE_MULT;
    }



    FILE * file = fopen(argv[2], "rb");
    fseek(file, 0, SEEK_END);
    u32 size = ftell(file);
    rewind(file);
    u8* buffer = (u8*)malloc(sizeof(u8) * (size + 1));
    u32 result = fread(buffer, 1, size, file);

    buffer[result] = 0;

    char* stream = (char*)buffer;



    // stream POSITION start
    // the numerical prefix is assumed to be that of the previous unless stated otherwise
    // same for length [S,T,I,Q,H,W] (I for eIghth)

    // [position]: ([note char][optional #]?[optional octave number]?[optional duration]?)+
    PQ* events = pq_init();
    //
    parse_stream(stream, events);

    // in per minute lol
    u16 BPM = 60;


    f64* run = left;
    f64* rrun = right;


    f64 fortyeighths = (f64)(BPM * 4 * 3) / (f64)(SAMPLE_FREQUENCY * 60);


    for (u32 i = 0; i < NUM_SAMPLES; i++) {

        u64 event = pq_peek(events);
        // so, empty
        if(event != MAX_U64) {

            u16 current_beat = (f64)(i) * fortyeighths;

            u64 event_beat = event >> 48;
            u64 event_index = event & MAX_U16;

            if (current_beat >= event_beat) {
                pq_pop(events);
                if (PRESS_BIT & event) {
                    voice_press(piano + event_index);
                } else {
                    voice_release(piano + event_index);
                }
            }
        }

        for (u16 i = 0; i < PIANO_KEYS; i++) {
            voice_step(piano + i, run, rrun);
        }

        run++;
        rrun++;
    }



    // idk, but lets scale it such that the highest peak is at this amplitude

    f64 min = *left;
    f64 max = *left;
    f64 value = *left;

    l = left; 
    r = right;
    for ( ; r < right + NUM_SAMPLES;)  {
        value = *r;
        if (value < min)
            min = value;
        else if (value > max)
            max = value;

        value = *l;
        if (value < min)
            min = value;
        else if (value > max)
            max = value;

        l++;
        r++;
    }

    printf("min %f max %f\n", min, max);

    if (min * -1.0 > max)
        max = -1.0 * min;

    f64 scaled_amplitude = effective_amplitude / max;
    printf("scaled amp %f\n", scaled_amplitude);

    // scale entire thing down
    r = right;
    l = left;
    for ( ; l < left + NUM_SAMPLES;) {
        *l *= scaled_amplitude;
        *r *= scaled_amplitude;
        l++;
        r++;
    }


    u8* sampled_data = (u8*)(wav + sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk));
    // DO NOT pass values not between -1 and 1 to this
    write_samples(left, right, sampled_data);


    // E, F, F#, G, G#, A, A#, B, C, C#, D, D#

    file = fopen("m04_arpeggio.wav" , "wb");

    if (!file)
        return 1;
    fwrite((const void *)wav,  sizeof(u8), REAL_FILE_SIZE, file);

    fclose(file);

    sf_free();

    free(wav);
    free(left);

    return 0;
}

