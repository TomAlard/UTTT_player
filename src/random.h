#ifndef UTTT2_RANDOM_H
#define UTTT2_RANDOM_H

#include <stdint.h>

typedef struct RNG {
    uint64_t state;
    uint64_t inc;
} RNG;

void seedRNG(RNG* rng, uint64_t init_state, uint64_t init_seq);

uint8_t generateBoundedRandomNumber(RNG* rng, uint8_t bound);

#endif //UTTT2_RANDOM_H
