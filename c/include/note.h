#ifndef NOTE_H
#define NOTE_H

#include "types.h"

// not even 16th notes, more like 48th notes

static const u16 STTH = 3;
static const u16 TRPL = 4;
static const u16 ETH = 6;
static const u16 QTR = 12;
static const u16 HLF = 24;
static const u16 WHL = 48;

typedef struct Note {
    u16 index; // temporary, will indicate pitch later
    u16 start; // also in 48th notes
    u16 duration; // in 48th notes
    u16 finish; // also in 48th notes, not necessary initialized
} Note;

Note* note_init(u16 index, u16 start, u16 duration);

#endif
