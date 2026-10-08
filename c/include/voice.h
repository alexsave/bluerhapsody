
#ifndef VOICE_H
#define VOICE_H


// this is like a single piano press key in a single row in Ableton. I think

static const u8 STAGE_OFF = 0;
static const u8 STAGE_ATTACK = 1;
static const u8 STAGE_DECAY = 2;
static const u8 STAGE_SUSTAIN = 3;
static const u8 STAGE_RELEASE = 4;

static const u8 TYPE_SIN = 0;
static const u8 TYPE_TRIANGLE = 1;
static const u8 TYPE_SQUARE = 2;
static const u8 TYPE_SAWTOOTH = 3;

typedef struct LRSample {
    f64 left;
    f64 right;
} LRSample;

typedef struct Voice {
    f64 phase; // like where we are in the "circle", this is the angle
    u8 envelope_stage;
    f64 time_into_stage_ms;


    // constants
    f64 sample_rate;

    f64 voice_frequency; // constant?
    f64 attack_ms;
    f64 decay_ms;
    f64 sustain_ratio;
    f64 release_ms;

    u8 wave_type;

    f64 attack_start;
    f64 release_start;
    
    //-1 left, 1 right
    f64 pan_position;

    f64 pan_start;
    f64 pan_end;
    f64 pan_ms;
    f64 time_into_pan_ms;

    f64 phase_bump;

} Voice;


// for this thing, I want to be able to define like a wave type, of a certain frequency, of a certain ADSR
// then also be able to press and release and press and release

// the usual suspects
Voice* voice_init(u32 sample_rate, u8 wave_type, f64 frequency, f64 attack_ms, f64 decay_ms, f64 sustain_ratio, f64 release_ms);

// intialize attack
void voice_press(Voice* voice);

// start release
void voice_release(Voice* voice);

// would be nice to take something like "samples since start", as constnatly adding to phase will hurt
// but jumping phase will also cause gaps if we change frequency so this is better
LRSample voice_step(Voice* voice);

void voice_pan(Voice* voice, f64 to, f64 pan_ms);

void voice_free(Voice* voice);


#endif

