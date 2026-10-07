#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "types.h"
#include "note.h"
#include "meter.h"
#include "wav.h"
#include "voice.h"

#include "fft.h"

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
    u16 A0 = 0;
    u16 A1 = 11;
    u16 A2 = 23;
    u16 A3 = 35;
    u16 A4 = 47;
    u16 AS4 = 48;
    u16 B4 = 49;

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

    u32 note_count = 2;

    Note * notes = calloc(note_count, sizeof(Note));

    //(*notes)[0] = { .index = A4, .start = 16, .duration = 1};
    //notes[1] = { .index = AS4, .start = 32, .duration = 1};
    *(notes + 0) = *note_init(A4, 16, 1);
    *(notes + 1) = *note_init(AS4, 32, 1);


    for (u32 i = 0; i < note_count; i++) {
        Note* n = notes + i;
        n->finish = n->start + n->duration;
    }

    // in per minute lol
    u16 BPM = 120;


    // only goes up
    u16 press_index = 0;
    u16 release_index = 0;


    f64* run = left;
    f64* rrun = right;

    //SAMPLE_FREQUENCY * BPM / 60 

    //f64 samples_per_sixteenth = SAMPLE_FREQUENCY * 60 / BPM / 4;

    f64 sixteenths_per_sample = (f64)(BPM * 4) / (f64)(SAMPLE_FREQUENCY * 60);

    //i / samples_per_sixteenth

    //i * BPM * 4 / SAMPLE_FREQUENCY / 60;


    for (u32 i = 0; i < NUM_SAMPLES; i++) {

        // in 1/16th notes
        // truncated to last 1/16th note
        // that 60 comes from 60 s per min, the 4 comes from 4 16th in a quarter note
        u16 current_beat = (f64)(i) * sixteenths_per_sample;
        //printf("current beat %d i %d\n", current_beat, i);

        while (press_index < note_count && notes[press_index].start == current_beat) {
            voice_press(piano + notes[press_index].index);
            press_index++;
        } 

        while (release_index < note_count && notes[release_index].finish == current_beat) {
            voice_release(piano + notes[release_index].index);
            release_index++;
        } 

        if (press_index < note_count && notes[press_index].start < current_beat ){
            printf("somehow we misssed a note, might be out of order start %d %d\n", current_beat, press_index);
            exit(1);
        }
        if( release_index < note_count && notes[release_index].finish < current_beat) {
            printf("somehow we misssed a note, might be out of order finish %d\n", current_beat);
            exit(1);
        }

        for (u16 i = 0; i < PIANO_KEYS; i++) {
            LRSample lrs = voice_step(piano + i);
            *run += lrs.left;
            *rrun += lrs.right;

        }

        run++;
        rrun++;
    }



    // idk, but lets scale it such that the highest peak is at this amplitude

    f64 min = *left;
    f64 max = *left;
    f64 value = *left;

    for (f64 * r = right; r < right + NUM_SAMPLES; r++) {
        value = *r;
        if (value < min)
            min = value;
        else if (value > max)
            max = value;
    }
    printf("min %f max %f\n", min, max);

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
    for (r = right; r < right + NUM_SAMPLES; r++)
        *r *= scaled_amplitude;


    u8* sampled_data = (u8*)(wav + sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk));
    // DO NOT pass values not between -1 and 1 to this
    write_samples(left, right, sampled_data);


    // E, F, F#, G, G#, A, A#, B, C, C#, D, D#

    FILE * file = fopen("m04_arpeggio.wav" , "wb");

    if (!file)
        return 1;
    fwrite((const void *)wav,  sizeof(u8), REAL_FILE_SIZE, file);

    fclose(file);


    free(wav);
    free(left);

    return 0;
}

