#ifndef UTTT2_LINEAR_H
#define UTTT2_LINEAR_H

#include <stdint.h>
#include <immintrin.h>
#include "parameters.h"


inline __attribute__((always_inline)) void m256_add_dpbusd_epi32(__m256i* acc, __m256i a, __m256i b) {
    __m256i one = _mm256_set1_epi16(1);
    __m256i product0 = _mm256_maddubs_epi16(a, b);
    product0 = _mm256_madd_epi16(product0, one);
    *acc = _mm256_add_epi32(*acc, product0);
}


inline __attribute__((always_inline)) float applyLinear256_1(__m256i regs[16]) {
    __m256i sum = _mm256_setzero_si256();
    for (int i = 0; i < 8; i++) {
        m256_add_dpbusd_epi32(&sum, regs[i], _mm256_load_si256((__m256i*) &outputWeights[i * 32]));
    }
    __m128i sum128lo = _mm256_castsi256_si128(sum);
    __m128i sum128hi = _mm256_extracti128_si256(sum, 1);
    __m128i sum128 = _mm_add_epi32(sum128lo, sum128hi);
    int output[4];
    _mm_store_si128((__m128i*) &output[0], sum128);
    int result = output[0] + output[1] + output[2] + output[3];
    return (float)result * (1.0f / (127*64));
}

#endif //UTTT2_LINEAR_H
