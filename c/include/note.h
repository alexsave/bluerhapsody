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

static const u16 A0 = 0;
static const u16 A1 = 11; 
static const u16 B1 = 13; 
static const u16 C1 = 14; 
static const u16 CS1 = 15; 
static const u16 A2 = 23; 
static const u16 C2 = 26; 
static const u16 CS2 = 27; 
static const u16 G2 = 33; 
static const u16 GS2 = 34; 

static const u16 A3 = 35; 
static const u16 B3 = 37; 
static const u16 C3 = 38; 
static const u16 CS3 = 39; 
static const u16 E3 = 42; 
static const u16 G3 = 45; 
static const u16 GS3 = 46; 

static const u16 A4 = 47; 
static const u16 AS4 = 48; 
static const u16 B4 = 49; 
static const u16 C4 = 50; 
static const u16 CS4 = 51; 
static const u16 D4 = 52; 
static const u16 DS4 = 53; 
static const u16 E4 = 54; 

typedef struct Note {
    u16 index; // temporary, will indicate pitch later
    u16 start; // also in 48th notes
    u16 duration; // in 48th notes
    u16 finish; // also in 48th notes, not necessary initialized
} Note;

Note* note_init(u16 index, u16 start, u16 duration);

#endif
