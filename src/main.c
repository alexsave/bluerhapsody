#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
 
static const u32 FREQUENCY = 48000;
static const u16 SAMPLE_BITS = 16;
static const u16 DURATION_S = 1;

static const u32 DATA_BYTES = (FREQUENCY * DURATION_S * SAMPLE_BITS) / 8;
static const u32 REAL_FILE_SIZE = sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk) + DATA_BYTES;

int main(int argc, char* argv[]){

    void* wav = malloc(REAL_FILE_SIZE);
    
    RiffChunk * rf = (RiffChunk*)wav;
    rf->fileTypeBlocID = (((((0x46 << 8) + 0x46) << 8) + 0x49) << 8) + 0x52;
    

    rf->fileFormatID = (((((0x45 << 8) + 0x56) << 8) + 0x41) << 8) + 0x57;

    FormatChunk * fc = (FormatChunk*)(wav + sizeof(RiffChunk));
    fc->formatBlocID = (((((0x20 << 8) + 0x74) << 8) + 0x6D) << 8) + 0x66;
    fc->blocSize = 16;

    fc->audioFormat = 1;
    fc->nbrChannels = 1;
    fc->frequency = FREQUENCY;
    fc->bitsPerSample = SAMPLE_BITS;

    fc->bytePerBloc = (fc->nbrChannels * SAMPLE_BITS / 8);
    fc->bytePerSec = FREQUENCY * fc->bytePerBloc;

    DataChunk * dc = (DataChunk*)(wav + sizeof(RiffChunk) + sizeof(FormatChunk));
    dc->dataBlocID = (((((0x61 << 8) + 0x74) << 8) + 0x61) << 8) + 0x64;

    dc->dataSize = DATA_BYTES;

    // only FileFormatID from RiffChunk is used here
    // "Overall file size minus 8 bytes"
    rf->fileSize = REAL_FILE_SIZE - 8;
    
    u8* sampledData = (u8*)(wav + sizeof(RiffChunk) + sizeof(FormatChunk) + sizeof(DataChunk));

    // now we can do fun stuff with sampled data
    
    


    FILE * file;
    file = fopen("1a.wav" , "wb");

    if (!file)
        return 1;

    fwrite((const void *)wav,  sizeof(u8), REAL_FILE_SIZE, file);

    fclose(file);

    free(wav);

    return 0;
}

