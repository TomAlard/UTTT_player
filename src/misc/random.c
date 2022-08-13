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


void shuffle(int* array, int n, RNG* rng) {
    uint32_t maxRandomNumber = (1L << 32) - 1;
    for (int i = 0; i < n - 1; i++) {
        uint32_t j = i + generateRandomNumber(rng) / (maxRandomNumber / (n - i) + 1);
        int t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}