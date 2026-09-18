#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "types.h"
#include "wasm.h"
#include "meter.h"
#include "wav.h"
#include "fft.h"

/// u32 is smaple count
WasmChannels* wasm_get_channels(u8* buffer) {
    // the allocations will be freed by JS... i hope

    WasmChannels * wasm_channels = malloc(sizeof(WasmChannels));
            
    FormatChunk* fc = (FormatChunk*)(buffer + sizeof(RiffChunk));

    u16 channel_count = fc->nbrChannels;
    u32 frequency = fc->frequency;
    u16 sample_bits = fc->bitsPerSample;

    //exit(1);

    DataChunk* dc = (DataChunk*)(buffer + sizeof(RiffChunk) + sizeof(FormatChunk));

    u32 data_bytes = dc->dataSize;
    u8* samples = dc->sampledData;

    // per channel
    u32 sample_count = data_bytes / (sample_bits / 8) / channel_count;

    printf("sample bits %d channel # %d sample_count %d\n", sample_bits, channel_count, sample_count);

    f64* left_buffer = malloc(sample_count * sizeof(f64));
    f64* right_buffer = malloc(sample_count * sizeof(f64));

    wasm_channels->sample_count = sample_count;
    wasm_channels->frequency = frequency;
    wasm_channels->left_channel = left_buffer;
    wasm_channels->right_channel = right_buffer;

    copy_samples(samples, sample_count, fc, left_buffer, right_buffer);

    return wasm_channels;
}

void set_frequencies(f64* frequency_array, f64 frequency, f64 bin_count) {
    // bin count or sample count? idk
    for (u32 i = 0; i < bin_count; i++) {
        frequency_array[i] = (f64)i * frequency / bin_count;
    }
}

void set_magnitudes(f64* mag_array, f64 bin_count, Cpx* fft_array) {
    for (u32 i = 0; i < bin_count; i++) 
        mag_array[i] = pow((fft_array[i].re * fft_array[i].re) + (fft_array[i].im * fft_array[i].im), 0.5);
}

// 0 for window samples is special case, just do fft over entire thing
Spectra * wasm_spectra(u8* file_buffer, u64 window_samples) {
    WasmChannels* wc = wasm_get_channels(file_buffer);


    if (window_samples == 0){
        // how many samples are in a window, basically 
        u64 bin_count;
        Cpx * fft_array = fft(wc->left_channel, wc->sample_count, &bin_count);

        // 1 for frequency, 1 for amplitude
        Spectra * spectra = malloc(sizeof(Spectra) + (1+1)*bin_count*sizeof(f64));
        spectra->sample_count = wc->sample_count;
        spectra->sample_rate = wc->frequency;
        spectra->window_samples = bin_count;
        spectra->num_windows = 1;

        f64* start = spectra->data;

        set_frequencies(start, wc->frequency, bin_count);
        start += bin_count;

        set_magnitudes(start, bin_count, fft_array);

        free(fft_array);
        
        return spectra;
    }

    //printf("getting fft windows\n");
    Cpx* out_cpx = 0;
    
    u64 window_count = fft_windows(wc->left_channel, wc->sample_count, &window_samples, &out_cpx);
    //printf("got fft windows\n");

    // now put it into a spectra

    Spectra * spectra = malloc(sizeof(Spectra) + (1+window_count) * window_samples * sizeof(f64));
    spectra->sample_count = wc->sample_count;
    spectra->sample_rate = wc->frequency;
    spectra->window_samples = window_samples;
    spectra->num_windows = window_count;

    f64* start = spectra->data;
    //printf("copying frequencies\n");
    set_frequencies(start, wc->frequency, window_samples);
    start += window_samples;

    //printf("setting magnitudes\n");
    for (u32 i = 0; i < window_count; i++){
        set_magnitudes(start, window_samples, out_cpx + window_samples * i);
        
        start += window_samples;
    }
    //printf("set magnitudes\n");
    


    return spectra;

    // otherwise much more intersting
}


FreqMag * wasm_fft(u8* file_buffer) {
    WasmChannels* wc = wasm_get_channels(file_buffer);

    u64 bin_count;

    Cpx * fft_array = fft(wc->left_channel, wc->sample_count, &bin_count);

    // return magntitudes and frequency array?

    FreqMag * ret = malloc(sizeof(FreqMag) * wc->sample_count);

    for (u32 i = 0; i < wc->sample_count; i++) {
        ret[i].mag = (fft_array[i].re * fft_array[i].re) + (fft_array[i].im * fft_array[i].im);
        ret[i].freq = (f64)i * (f64)wc->frequency / (f64)bin_count;
    }

    
    return ret;

}
