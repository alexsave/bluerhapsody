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
static const u16 FS0 = 9;

static const u16 A1 = 12; 
static const u16 B1 = 14; 
static const u16 C1 = 15; 
static const u16 CS1 = 16; 

static const u16 A2 = 24; 
static const u16 C2 = 27; 
static const u16 CS2 = 28; 
static const u16 FS1 = 32;
static const u16 G2 = 33; 
static const u16 GS2 = 35; 

static const u16 A3 = 36; 
static const u16 AS3 = 37; 
static const u16 B3 = 38; 
static const u16 C3 = 39; 
static const u16 CS3 = 40; 
static const u16 D3 = 41;
static const u16 DS3 = 42; 
static const u16 E3 = 43; 
static const u16 F3 = 44; 
static const u16 FS3 = 45; 
static const u16 G3 = 46; 
static const u16 GS3 = 47; 

static const u16 A4 = 48; 
static const u16 AS4 = 49; 
static const u16 B4 = 50; 
static const u16 C4 = 51; 
static const u16 CS4 = 52; 
static const u16 D4 = 53; 
static const u16 DS4 = 54; 
static const u16 E4 = 55; 

typedef struct Note {
    u16 index; // temporary, will indicate pitch later
    u16 start; // also in 48th notes
    u16 duration; // in 48th notes
    u16 finish; // also in 48th notes, not necessary initialized
} Note;

Note* note_init(u16 index, u16 start, u16 duration);

#endif
