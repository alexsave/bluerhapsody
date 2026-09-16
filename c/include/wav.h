#ifndef WAV_H
#define WAV_H

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

#endif

