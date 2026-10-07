#ifndef NOTE_H
#define NOTE_H

#include "types.h"

typedef struct Note {
    u16 index; // temporary, will indicate pitch later
    u16 start; // also in 16th notes
    u16 duration; // in 16th notes
    u16 finish; // also in 16th notes, not necessary initialized
} Note;

Note* note_init(u16 index, u16 start, u16 duration);

#endif
