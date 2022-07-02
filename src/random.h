#ifndef UTTT2_RANDOM_H
#define UTTT2_RANDOM_H

#include <stdint.h>

typedef struct pcg32_random_t {
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

uint32_t pcg32_random_r(pcg32_random_t* rng);

void pcg32_srandom_r(pcg32_random_t* rng, uint64_t init_state, uint64_t init_seq);

uint32_t pcg32_boundedrand_r(pcg32_random_t* rng, uint32_t bound);

#endif //UTTT2_RANDOM_H
