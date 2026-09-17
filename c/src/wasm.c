#include <stdlib.h>
#include <stdio.h>

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

FreqMag * wasm_fft(u8* file_buffer) {
    WasmChannels* wc = wasm_get_channels(file_buffer);

    u32 bin_count;

    Cpx * fft_array = fft(wc->left_channel, wc->sample_count, &bin_count);

    // return magntitudes and frequency array?

    FreqMag * ret = malloc(sizeof(FreqMag) * wc->sample_count);

    for (u32 i = 0; i < wc->sample_count; i++) {
        ret[i].mag = (fft_array[i].re * fft_array[i].re) + (fft_array[i].im * fft_array[i].im);
        ret[i].freq = (f64)i * (f64)wc->frequency / (f64)bin_count;
    }

    
    return ret;

}
