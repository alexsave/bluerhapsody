#ifndef PARSER_H
#define PARSER_H

#include "pq.h"
#include "note.h"

static const u64 PRESS_BIT = 1 << 16;

void parse_stream(char* stream, PQ* events);

#endif

