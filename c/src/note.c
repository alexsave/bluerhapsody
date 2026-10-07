#import <stdlib.h>

#import "types.h"
#import "note.h"

Note* note_init(u16 index, u16 start, u16 duration) {
    Note* note = malloc(sizeof(Note));
    note->index = index;
    note->start = start;
    note->duration = duration;
    note->finish = start + duration;
    return note;
}

