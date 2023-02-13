#ifndef UTTT2_CLIPPED_RELU_H
#define UTTT2_CLIPPED_RELU_H

#include <stdint.h>
#include <immintrin.h>

inline __attribute__((always_inline)) void applyClippedReLU256(const int16_t* input, int8_t* output) {
    __m256i zero = _mm256_setzero_si256();
    for (int i = 0; i < 8; i++) {
        __m256i in0 = _mm256_load_si256((__m256i*) &input[i*32]);
        __m256i in1 = _mm256_load_si256((__m256i*) &input[i*32 + 16]);
        __m256i result = _mm256_permute4x64_epi64(_mm256_max_epi8(_mm256_packs_epi16(in0, in1), zero), 0b11011000);
        _mm256_store_si256((__m256i*) &output[i * 32], result);
    }
}

#endif //UTTT2_CLIPPED_RELU_H
