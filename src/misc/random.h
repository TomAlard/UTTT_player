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

uint8_t generateBoundedRandomNumber(RNG* rng, uint8_t bound);

void shuffle(int* array, size_t n, RNG* rng);

#endif //UTTT2_RANDOM_H
