#ifndef PQ_H
#define PQ_H

#include "types.h"

// maybe this can be PQ
typedef struct PQ {
    u32 current; // or should this point direclty into array?
    u64* heap;
} PQ;

static const u64 INITIAL_CAPACITY = 1024;
static const u64 CAPACITY_INDEX = 0;

PQ* pq_init();

void pq_push(PQ* pq, u64 event);

u64 pq_peek(PQ* pq);

u64 pq_pop(PQ* pq);

uint8_t pq_is_empty(PQ* pq);

void pq_free(PQ* pq);

#endif
