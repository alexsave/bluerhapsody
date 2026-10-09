#import <stdlib.h>
#import <stdio.h>
#import <math.h>

#import "types.h"
#import "constants.h"
#import "shape.h"
#import "voice.h"

Voice* voice_init(u32 sample_rate, u8 wave_type, f64 frequency, f64 attack_ms, f64 decay_ms, f64 sustain_ratio, f64 release_ms) {
    Voice* voice = malloc(sizeof(Voice));

    voice->phase = 0.0;
    voice->sum_phase = 0.0;
    voice->envelope_stage = STAGE_OFF;
    voice->time_into_stage_ms = 0.0;

    voice->sample_rate = sample_rate;


    voice->voice_frequency = frequency;
    voice->attack_ms = attack_ms;
    voice->decay_ms = decay_ms;
    voice->sustain_ratio = sustain_ratio;
    voice->release_ms = release_ms;
    
    voice->wave_type = wave_type;

    // this is like at what amplitude did we hit the button
    voice->attack_start = 0.0;

    voice->pan_position = 0.0;
    voice->pan_ms = 0.0;
    voice->pan_start = 0.0;
    voice->pan_end = 0.0;
    voice->time_into_pan_ms = 0.0;

    voice->phase_bump = 2.0 * M_PI * voice->voice_frequency / voice->sample_rate;

    return voice;
}

void voice_set_type(Voice* voice, u8 wave_type) {
    voice->wave_type = wave_type;
}

// like current amplitude irregardless of freq, only ADSR
f64 current_ampl(Voice* voice){
    if (voice->envelope_stage == STAGE_OFF) 
        return 0.0;
    if (voice->envelope_stage == STAGE_ATTACK) 
        return voice->attack_start + (1.0-voice->attack_start) * (voice->time_into_stage_ms) / (voice->attack_ms);
    if (voice->envelope_stage == STAGE_DECAY) 
        return (1.0 - (1.0 - voice->sustain_ratio) * (voice->time_into_stage_ms / voice->decay_ms));
    if (voice->envelope_stage == STAGE_SUSTAIN) 
        return voice->sustain_ratio;

    if (voice->envelope_stage == STAGE_RELEASE) 
        return (voice->sustain_ratio - (voice->sustain_ratio * voice->time_into_stage_ms / voice->release_ms));

}

// intialize attack
void voice_press(Voice* voice) {
    voice->attack_start = current_ampl(voice);
    voice->envelope_stage = STAGE_ATTACK;
    voice->time_into_stage_ms = 0.0;
}

// start release
void voice_release(Voice* voice) {
    voice->release_start = current_ampl(voice);
    voice->envelope_stage = STAGE_RELEASE;
    voice->time_into_stage_ms = 0.0;
}

void pan_update(Voice* voice) {
    if(voice->pan_ms != 0.0) {
        // ok now another phase, stereo
        voice->time_into_pan_ms += 1000.0 / voice->sample_rate;
        if(voice->time_into_pan_ms >= voice->pan_ms) {
            // stop it
            voice->pan_ms = 0.0;
            voice->time_into_pan_ms = 0.0;
        } else {
            // not done yet
            voice->pan_position = voice->pan_start + (voice->pan_end - voice->pan_start) * (voice->time_into_pan_ms / voice->pan_ms);
        }
    }
    
}

// one sample rate at a time
// add samples to left and rigth
void voice_step(Voice* voice, f64* left, f64* right) {
    // frequency and sample_rate
    // lets see
    // if frequency was 2hz, and sample rate was 8hz
    // the phase would go 0, pi/2, pi, 3pi/2, 2pi
    // ok if your note frequency was HIGHER this whould hcang emore
    // if sample rate was higher this would change less

    // each is Hz
    //printf("update by %f (%f/%f)\n", (voice->voice_frequency / voice->sample_rate), voice->voice_frequency, voice->sample_rate);
    // wait it woulb e better to keep track of how many samples we are into the 

    // default really high
    // default same stage

    if (voice->envelope_stage == STAGE_OFF) {
        // nothing lol
        pan_update(voice);

        return;

    } 

    voice->phase += voice->phase_bump;
    voice->sum_phase += voice->phase_bump;
    if (voice->phase >= TWO_PI) 
        voice->phase -= TWO_PI;

    voice->time_into_stage_ms += 1000.0 / voice->sample_rate;
    f64 end_of_stage_ms = 1000*60*60*24;
    u8 next_stage = voice->envelope_stage;

    if (voice->envelope_stage == STAGE_ATTACK) {
        end_of_stage_ms = voice->attack_ms;
        next_stage = STAGE_DECAY;
    } else if (voice->envelope_stage == STAGE_DECAY) {
        end_of_stage_ms = voice->decay_ms;
        next_stage = STAGE_SUSTAIN;
    } else if (voice->envelope_stage == STAGE_SUSTAIN) {
        //end_of_stage_ms = voice->decay_ms;
        // it ends when yo let go
    } else if (voice->envelope_stage == STAGE_RELEASE) {
        end_of_stage_ms = voice->release_ms;
        next_stage = STAGE_OFF;
    }

    if (voice->time_into_stage_ms >= end_of_stage_ms) {
        //swap the stage
        voice->envelope_stage = next_stage;
        voice->time_into_stage_ms = 0.0;
    }

    // ok now emit
    // for now just do sin wave

    //printf("current_ampl %f raw %f phase %f\n", current_ampl(voice), sin(voice->phase), voice->phase);

    pan_update(voice);

    f64 raw;
    if (voice->wave_type == SIN) raw = sine(voice->phase);
    else if (voice->wave_type == TRIANGLE_SUM) raw = triangle(voice->phase, 8);
    else if (voice->wave_type == TRIANGLE_NAIVE) raw = triangle_naive(voice->phase);
    else if (voice->wave_type == SQUARE_SUM) raw = square(voice->phase, 3);
    else if (voice->wave_type == SQUARE_NAIVE) raw = square_naive(voice->phase);
    else if (voice->wave_type == SQUARE_POLYBLEP) raw = square_polyblep(voice->phase, voice->phase_bump);
    else if (voice->wave_type == SAW_SUM) raw = sawtooth(voice->phase, 8);
    else if (voice->wave_type == SAW_NAIVE) raw = saw_naive(voice->phase);
    else if (voice->wave_type == SAW_POLYBLEP) raw = saw_polyblep(voice->phase, voice->phase_bump);
    else if (voice->wave_type == WHITE_NOISE) raw = white(voice->phase);
    else if (voice->wave_type == BITCRUSH) raw = bitcrush(voice->sum_phase);

    // ok now lets try to be dumb about it

    f64 unpanned = raw * current_ampl(voice);

    // linear -> -1.0 means left * 1, right * 0
    // linear -> 1.0 means left * 0, right * 1

    //-1 is like max left, so  first

    f64 angle = ((voice->pan_position + 1.0) * M_PI / 4.0);

    // maybe it would be easier to do 0 to 1?
    // nah -1 and 1 makes more sense
    *left += unpanned * cos(angle);
    *right += unpanned * sin(angle);
}

void voice_pan(Voice* voice, f64 to, f64 pan_ms) {
    voice->pan_start = voice->pan_position;
    voice->pan_end = to;
    voice->time_into_pan_ms = 0.0;
    voice->pan_ms = pan_ms;
}

void voice_free(Voice* voice) {
    free(voice);
}


