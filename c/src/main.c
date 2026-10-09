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
#include "format.h"
#include "pq.h"

static const u16 BPM = 60;

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

    if (argc <= 2)
        return 0;


    sf_reserve();

    f32 gain = -12.0;


    // power is correlated to square of amplitude, thus if you want to 10x
    const f64 AMP_MULT = pow(10.0, 1.0/20.0);
    //printf("%lf\n", AMP_MULT);

    f64 effective_amplitude = pow(AMP_MULT, gain);

    // frequency means that it does 440 full rotations through circle in 1 second
    // thus y = sin(2pi*x)
    // thus y = sin(440 * 2pi*x)
    // if we step through sample by sample, x will be i/SAMPLE_RATE

    // i will go all the way up to SAMPLE_RATE * DURATION

    // now we can do fun stuff with sampled data

    const f64 SEMITONE_MULT = pow(2.0, 1.0/12.0);

    // 16 s in I hear something - roughly 32Hz
    // 100x, 1:24 loses me
    // my hearing is roughly 32Hz to 16.8kHz wow

    u16 PIANO_KEYS = 120;

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

        temp = voice_init(SAMPLE_RATE, SAW_POLYBLEP, note, 50, 50, .5, 100);
        memcpy(piano + i, temp, sizeof(Voice));
        note *= SEMITONE_MULT;
    }

    PQ* events = pq_init();
    //
    u64 beats = parse_stream(events, argv[2]);

    // in per minute lol

    // bpm = beat / 60s
    // bpm / beat * 60 = s

    u64 sec = (5 * beats) / BPM;

    // headroom, let releases play
    sec += 2;

    u32 num_samples = sec * SAMPLE_RATE;

    f64* left = calloc(sizeof(f64), num_samples);
    f64* right = calloc(sizeof(f64), num_samples);

    // now we can make files, now that we have seconds
    void* wav = write_headers(sec);

    f64* run = left;
    f64* rrun = right;

    f64* l = left;
    f64* r = right;

    f64 fortyeighths = (f64)(BPM * 4 * 3) / (f64)(SAMPLE_RATE * 60);

    for (u32 i = 0; i < num_samples; i++) {

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

        for (u16 i = 0; i < PIANO_KEYS; i++) 
            voice_step(piano + i, run, rrun);

        run++;
        rrun++;
    }

    // idk, but lets scale it such that the highest peak is at this amplitude

    f64 min = *left;
    f64 max = *left;
    f64 value = *left;

    l = left; 
    r = right;
    for ( ; r < right + num_samples;)  {
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
    for ( ; l < left + num_samples;) {
        *l *= scaled_amplitude;
        *r *= scaled_amplitude;
        l++;
        r++;
    }

    write_wav(left, right, wav, "m05_osc_compare.wav", sec);

    sf_free();

    free(wav);
    free(left);
    free(right);

    return 0;
}

