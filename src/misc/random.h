#ifndef UTTT2_RANDOM_H
#define UTTT2_RANDOM_H

#include <stdint.h>
#include <stddef.h>
#include "../board/square.h"

typedef struct RNG {
    uint64_t state;
    uint64_t inc;
} RNG;

void seedRNG(RNG* rng, uint64_t init_state, uint64_t init_seq);

void shuffle(int* array, int n, RNG* rng);

#endif //UTTT2_RANDOM_H
