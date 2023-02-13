#ifndef UTTT2_RNG_H
#define UTTT2_RNG_H

#include <stdint.h>

typedef struct RNG {
    uint64_t state;
    uint64_t inc;
} RNG;

inline __attribute__((always_inline)) uint32_t pcg32_random_r(RNG* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}


inline __attribute__((always_inline)) void seedRNG(RNG* rng, uint64_t init_state, uint64_t init_seq) {
    rng->state = 0U;
    rng->inc = (init_seq << 1u) | 1u;
    pcg32_random_r(rng);
    rng->state += init_state;
    pcg32_random_r(rng);
}


inline __attribute__((always_inline)) uint8_t generateBoundedRandomNumber(RNG* rng, uint8_t bound) {
    return ((uint64_t)pcg32_random_r(rng) * (uint64_t)bound) >> 32;
}
#endif //UTTT2_RNG_H
