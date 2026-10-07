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

#include "fft.h"
#include "pq.h"

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

    u32 note_count = 200;

    Note * notes = calloc(note_count, sizeof(Note));

    // stream POSITION start
    // the numerical prefix is assumed to be that of the previous unless stated otherwise
    // same for length [S,T,I,Q,H,W] (I for eIghth)

    // [position]: ([note char][optional #]?[optional octave number]?[optional duration]?)+
    char* stream1 = "0: G#2T C#3 E G#2 C#3 E G#2 C#3 E G#2 C#3 E G#2 C#3 E G#2 C#3 E G#2 C#3 E G#2 C#3 E";
    char* stream2 = "0: (C#1W C#2) (B1 B2)";

    /**(notes + 2) = *note_init(GS2, 0, TRPL);
    *(notes + 3) = *note_init(CS3, 4, TRPL);
    *(notes + 4) = *note_init(E3, 8, TRPL);
    *(notes + 5) = *note_init(GS2, 12, TRPL);
    *(notes + 6) = *note_init(CS3, 16, TRPL);
    *(notes + 7) = *note_init(E3, 20, TRPL);
    *(notes + 8) = *note_init(GS2, 24, TRPL);
    *(notes + 9) = *note_init(CS3, 28, TRPL);
    *(notes + 10) = *note_init(E3, 32, TRPL);
    *(notes + 11) = *note_init(GS2, 36, TRPL);
    *(notes + 12) = *note_init(CS3, 40, TRPL);
    *(notes + 13) = *note_init(E3, 44, TRPL);*/

    // what do we need? a list of events start and finishes

    u8 in_paren = 0;

    char* ptr = stream1;

    u8 octave_n = 0;


    // in the format this is in quarter notes, not 48ths
    u64 start_position = 0;
    while(*ptr != ':') {
        start_position *= 10;
        start_position += (*ptr - '0');
        ptr++;
        if (*ptr == 0){
            printf("invalid format, no colon\n");
            exit(1);
        }
    }

    PQ* events = pq_init();

    // 1 << 16
    u64 PRESS_BIT = 1 << 16;

    
    // convert to 48ths
    start_position *= QTR;
    
    // step through char by char until the end

    u8 SPACE = 0;
    u8 NOTE = 1;
    u8 A_PAREN = 2;
    u8 B_PAREN = 2;

    u8 parser_state = SPACE;

    u8 note_mod = MAX_U8;
    u16 dur = 0;

    u64 current_beat = 0;
    u16 start_of_group = 0;
    u64 group_first_dur = 0;
    u64 group_first_start = 0;

    u8 first_of_group = 0;

    while(*ptr != 0) {
        char c = *ptr;
        if (c == '(' || c == ')' || c == ' ' || *(ptr+1) == 0) {
            printf("last note ended but %d %d\n", note_mod, dur);
            // last note definitely just ended
            if(note_mod != MAX_U8){
                u16 index = note_mod + octave_n* 12;
                if(dur != 0){
                    if(!in_paren){
                        u64 s = ((u64)current_beat << 48) | PRESS_BIT | index;
                        u64 f = ((u64)(current_beat+dur) << 48) | index;

                        printf("note detected, from %d to %d, index %d\n", current_beat, current_beat+dur, index);

                        pq_push(events, s);
                        pq_push(events, f);

                        current_beat += dur;
                    } else {
                        u64 s = ((u64)group_first_start << 48) | PRESS_BIT | index;
                        u64 f = ((u64)(group_first_start+dur) << 48) | index;

                        printf("note detected, from %d to %d, index %d\n", group_first_start, group_first_start+dur, index);

                        pq_push(events, s);
                        pq_push(events, f);
                    }
                }
            }

            note_mod = MAX_U8;
        }
        if (c == ' '){
            parser_state = SPACE;
            if(note_mod != MAX_U8)
                first_of_group = 0;
        } else if ((c >= 'A') && (c <= 'G')) {
            if (parser_state != SPACE && parser_state != A_PAREN) {
                printf("invalaid format, no space before note\n");
            }
            if (c == 'A')
                note_mod = 0;
            else if (c == 'B')
                note_mod = 2;
            else if (c == 'C')
                note_mod = 3;
            else if (c == 'D')
                note_mod = 5;
            else if (c == 'E')
                note_mod = 7;
            else if (c == 'F')
                note_mod = 8;
            else if (c == 'G')
                note_mod = 10;
            parser_state = NOTE; 
        } else if (c == '(') {
            if (parser_state != SPACE) {
                printf("invalid format, no space before parenthesis\n");
                exit(1);
            } 
            if (in_paren) {
                printf("invalid format, double parenthesis\n");
                exit(1);
            } 
            in_paren = 1;
            printf("setting group first start to %d\n", current_beat);
            group_first_start = current_beat;
            first_of_group = 1;
            group_first_dur = dur;
            printf("setting group first duration to %d\n", dur);
            parser_state = A_PAREN;
        } else if (c == ')') {
            if (!in_paren) {
                printf("invalid format, stray parenthesis\n");
                exit(1);
            } 
            in_paren = 0;
            current_beat += group_first_dur;
            parser_state = B_PAREN;
            dur = group_first_dur;
            printf("setting duration to gfd %d\n", group_first_dur);
            start_of_group = 0;
            note_mod = MAX_U8;
        } else if (c == 'b') {
            if (parser_state != NOTE) {
                printf("invalid format, stray #\n");
                exit(1);
            }
            note_mod = (note_mod + 12 - 1) % 12;
        } else if (c == '#') {
            if (parser_state != NOTE) {
                printf("invalid format, stray #\n");
                exit(1);
            }
            
            note_mod += 1;
        } else if ((c >= '0') && (c <= '9')) {
            if (parser_state != NOTE) {
                printf("invalid format, stray number\n");
                exit(1);
            }
            octave_n = (c - '0');
        } else if (c == 'S' || c == 'T' || c == 'I' || c == 'Q' || c == 'H' || c == 'W') {
            if (parser_state != NOTE) {
                printf("invalid format, stray duration\n");
                exit(1);
            }

            if (c == 'S')
                dur = STTH;
            else if (c == 'T')
                dur = TRPL;
            else if (c == 'I')
                dur = ETH;
            else if (c == 'Q')
                dur = QTR;
            else if (c == 'H')
                dur = HLF;
            else if (c == 'W')
                dur = WHL;

            //parser_state = TIME;

            if (in_paren && (first_of_group == 1)) {
                group_first_dur = dur;
                printf("first of group, setting gfd to dur %d\n", dur);
                first_of_group = 0;
            }
            // otherwise if you dont set the duration of the note atom, the gorup will adopt the previous duration
        }


        if (*(ptr+1) == 0) {
            if(note_mod != MAX_U8){
                u16 index = note_mod + octave_n* 12;
                if(dur != 0){
                    if(!in_paren){
                        u64 s = ((u64)current_beat << 48) | PRESS_BIT | index;
                        u64 f = ((u64)(current_beat+dur) << 48) | index;

                        printf("note detected, from %d to %d, index %d\n", current_beat, current_beat+dur, index);

                        pq_push(events, s);
                        pq_push(events, f);

                        current_beat += dur;
                    } else {
                        u64 s = ((u64)start_of_group << 48) | PRESS_BIT | index;
                        u64 f = ((u64)(start_of_group+dur) << 48) | index;

                        printf("note detected, from %d to %d, index %d\n", start_of_group, start_of_group+dur, index);

                        pq_push(events, s);
                        pq_push(events, f);
                    }
                }
            }

            note_mod = MAX_U8;
        }


        ptr++;

        ////if(*ptr == 0) {
        //
        //}



    }
    // incase there is no final space
    //u64 s = ((u64)current_beat << 48) | PRESS_BIT | (note_mod + octave_n * 12);
    //u64 f = ((u64)(current_beat+dur) << 48) | (note_mod + octave_n * 12);
    //
    ////pq_push(events, s);
    //pq_push(events, f);


    events = pq_init();

    parse_stream(stream1, events);
    parse_stream(stream2, events);


    //printf("start pos %llu\n", start_position);



    *(notes + 1) = *note_init(CS2, 0, WHL);
    *(notes + 0) = *note_init(CS1, 0, WHL);

    *(notes + 2) = *note_init(GS2, 0, TRPL);
    *(notes + 3) = *note_init(CS3, 4, TRPL);
    *(notes + 4) = *note_init(E3, 8, TRPL);
    *(notes + 5) = *note_init(GS2, 12, TRPL);
    *(notes + 6) = *note_init(CS3, 16, TRPL);
    *(notes + 7) = *note_init(E3, 20, TRPL);
    *(notes + 8) = *note_init(GS2, 24, TRPL);
    *(notes + 9) = *note_init(CS3, 28, TRPL);
    *(notes + 10) = *note_init(E3, 32, TRPL);
    *(notes + 11) = *note_init(GS2, 36, TRPL);
    *(notes + 12) = *note_init(CS3, 40, TRPL);
    *(notes + 13) = *note_init(E3, 44, TRPL);
    //*(notes + 14) = 0?/;
    //*(notes + 15) = note...


    /*for (u32 i = 0; i < note_count; i++) {
      Note* n = notes + i;
      n->finish = n->start + n->duration;

      u64 s = ((u64)n->start << 48) | PRESS_BIT | n->index;
      u64 f = ((u64)n->finish << 48) | n->index;

      pq_push(events, s);
      pq_push(events, f);

      }*/

    // in per minute lol
    u16 BPM = 60;


    // only goes up
    u16 press_index = 0;
    u16 release_index = 0;


    f64* run = left;
    f64* rrun = right;


    f64 fortyeighths = (f64)(BPM * 4 * 3) / (f64)(SAMPLE_FREQUENCY * 60);


    for (u32 i = 0; i < NUM_SAMPLES; i++) {

        if(!pq_is_empty(events)) {

            u16 current_beat = (f64)(i) * fortyeighths;

            u64 event = pq_peek(events);
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

