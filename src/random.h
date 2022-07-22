#ifndef UTTT2_RANDOM_H
#define UTTT2_RANDOM_H

#include <stdint.h>

typedef struct pcg32_random_t {
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

void pcg32_srandom_r(pcg32_random_t* rng, uint64_t init_state, uint64_t init_seq);

uint8_t pcg32_boundedrand_r(pcg32_random_t* rng, uint8_t bound);

#endif //UTTT2_RANDOM_H
