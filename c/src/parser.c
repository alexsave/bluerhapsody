#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "pq.h"
#include "types.h"
#include "constants.h"
#include "note.h"

void parse_stream(char* stream, PQ* events) {

    u8 in_paren = 0;

    char* ptr = stream;

    u8 octave_n = 0;


    // in the format this is in quarter notes, not 48ths


    // convert to 48ths

    // step through char by char until the end

    u8 SPACE = 0;
    u8 NOTE = 1;
    u8 A_PAREN = 2;
    u8 B_PAREN = 2;

    u8 parser_state = SPACE;

    u8 note_mod = MAX_U8;
    u16 dur = 0;

    u64 current_beat = 0;
    u16 start_of_group = 0;
    u64 group_first_dur = 0;
    u64 group_first_start = 0;

    u8 first_of_group = 0;
    u8 rest = 0;

    while(*ptr != 0) {
        char c = *ptr;
        if (c == '(' || c == ')' || c == ' ' || c == '|') {
            //printf("last note ended but %d %d\n", note_mod, dur);
            // last note definitely just ended
            if (rest) {
                current_beat += dur;
                rest = 0;
            } else if(note_mod != MAX_U8){
                u16 index = note_mod + octave_n* 12;
                if(dur != 0){
                    if(!in_paren){
                        u64 s = ((u64)current_beat << 48) | PRESS_BIT | index;
                        u64 f = ((u64)(current_beat+dur) << 48) | index;

                        //printf("note detected, from %d to %d, index %d\n", current_beat, current_beat+dur, index);

                        pq_push(events, s);
                        pq_push(events, f);

                        current_beat += dur;
                    } else {
                        u64 s = ((u64)group_first_start << 48) | PRESS_BIT | index;
                        u64 f = ((u64)(group_first_start+dur) << 48) | index;

                        //printf("note detected, from %d to %d, index %d\n", group_first_start, group_first_start+dur, index);

                        pq_push(events, s);
                        pq_push(events, f);
                    }
                }
            }

            note_mod = MAX_U8;
        }

        if (c == '|') {
            // that means that some beat # follows, and we should go there
            ptr++;
            u64 start_position = 0;
            while(*ptr != ':') {
                start_position *= 10;
                start_position += (*ptr - '0');
                ptr++;
                if (*ptr == 0){
                    printf("invalid format, no colon\n");
                    exit(1);
                }
            }
        
            current_beat = start_position * QTR;

        } else if (c == ' '){
            parser_state = SPACE;
            if(note_mod != MAX_U8)
                first_of_group = 0;
        } else if ((c >= 'A') && (c <= 'G')) {
            if (parser_state != SPACE && parser_state != A_PAREN) {
                printf("invalaid format, no space before note\n");
            }
            if (c == 'A')
                note_mod = 0;
            else if (c == 'B')
                note_mod = 2;
            else if (c == 'C')
                note_mod = 3;
            else if (c == 'D')
                note_mod = 5;
            else if (c == 'E')
                note_mod = 7;
            else if (c == 'F')
                note_mod = 8;
            else if (c == 'G')
                note_mod = 10;
            parser_state = NOTE;
        } else if (c == 'R') {
            // rest, takes a duration like a note
            rest = 1;
            parser_state = NOTE;
        } else if (c == '.') {
            // dotted
            dur += dur / 2;
            if (in_paren)
                group_first_dur = dur;
        } else if (c == '(') {
            if (parser_state != SPACE) {
                printf("invalid format, no space before parenthesis\n");
                exit(1);
            }
            if (in_paren) {
                printf("invalid format, double parenthesis\n");
                exit(1);
            }
            in_paren = 1;
            //printf("setting group first start to %d\n", current_beat);
            group_first_start = current_beat;
            first_of_group = 1;
            group_first_dur = dur;
            //printf("setting group first duration to %d\n", dur);
            parser_state = A_PAREN;
        } else if (c == ')') {
            if (!in_paren) {
                printf("invalid format, stray parenthesis\n");
                exit(1);
            }
            in_paren = 0;
            current_beat += group_first_dur;
            parser_state = B_PAREN;
            dur = group_first_dur;
            //printf("setting duration to gfd %d\n", group_first_dur);
            start_of_group = 0;
            note_mod = MAX_U8;


        } else if (c == 'b') {
            if (parser_state != NOTE) {
                printf("invalid format, stray #\n");
                exit(1);
            }
            note_mod = (note_mod + 12 - 1) % 12;
        } else if (c == '#') {
            if (parser_state != NOTE) {
                printf("invalid format, stray #\n");
                exit(1);
            }

            note_mod += 1;
        } else if ((c >= '0') && (c <= '9')) {
            if (parser_state != NOTE) {
                printf("invalid format, stray number\n");
                exit(1);
            }
            octave_n = (c - '0');
        } else if (c == 'S' || c == 'T' || c == 'I' || c == 'Q' || c == 'H' || c == 'W') {
            if (parser_state != NOTE) {
                printf("invalid format, stray duration\n");
                exit(1);
            }

            if (c == 'S')
                dur = STTH;
            else if (c == 'T')
                dur = TRPL;
            else if (c == 'I')
                dur = ETH;
            else if (c == 'Q')
                dur = QTR;
            else if (c == 'H')
                dur = HLF;
            else if (c == 'W')
                dur = WHL;

            //parser_state = TIME;

            if (in_paren && (first_of_group == 1)) {
                group_first_dur = dur;
                //printf("first of group, setting gfd to dur %d\n", dur);
                first_of_group = 0;
            }
            // otherwise if you dont set the duration of the note atom, the gorup will adopt the previous duration
        }


        ptr++;

    }

    if(note_mod != MAX_U8){
        u16 index = note_mod + octave_n* 12;
        if(dur != 0){
            if(!in_paren){
                u64 s = ((u64)current_beat << 48) | PRESS_BIT | index;
                u64 f = ((u64)(current_beat+dur) << 48) | index;

                //printf("note detected, from %d to %d, index %d\n", current_beat, current_beat+dur, index);

                pq_push(events, s);
                pq_push(events, f);

                current_beat += dur;
            } else {
                u64 s = ((u64)start_of_group << 48) | PRESS_BIT | index;
                u64 f = ((u64)(start_of_group+dur) << 48) | index;

                //printf("note detected, from %d to %d, index %d\n", start_of_group, start_of_group+dur, index);

                pq_push(events, s);
                pq_push(events, f);
            }
        }
    }


}
