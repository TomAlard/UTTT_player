#include "random.h"


uint32_t generateRandomNumber(RNG* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}


void seedRNG(RNG* rng, uint64_t init_state, uint64_t init_seq) {
    rng->state = 0U;
    rng->inc = (init_seq << 1u) | 1u;
    generateRandomNumber(rng);
    rng->state += init_state;
    generateRandomNumber(rng);
}


uint8_t generateBoundedRandomNumber(RNG* rng, uint8_t bound) {
    return ((uint64_t) generateRandomNumber(rng) * (uint64_t)bound) >> 32;
}