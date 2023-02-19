#ifndef UTTT2_CLIPPED_RELU_H
#define UTTT2_CLIPPED_RELU_H

#include <stdint.h>
#include <immintrin.h>

void applyClippedReLU256(const int16_t* input, int8_t* output) {
    __m256i zero = _mm256_setzero_si256();
    for (int i = 0; i < 8; i++) {
        __m256i in0 = _mm256_load_si256((__m256i*) &input[i*32]);
        __m256i in1 = _mm256_load_si256((__m256i*) &input[i*32 + 16]);
        __m256i result = _mm256_permute4x64_epi64(_mm256_max_epi8(_mm256_packs_epi16(in0, in1), zero), 0b11011000);
        _mm256_store_si256((__m256i*) &output[i * 32], result);
    }
}


void applyClippedReLU32(const int32_t* input, int8_t* output) {
    __m256i zero = _mm256_setzero_si256();
    __m256i control = _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0);
    __m256i in0 = _mm256_packs_epi32(_mm256_load_si256((__m256i*) &input[0]), _mm256_load_si256((__m256i*) &input[8]));
    __m256i in1 = _mm256_packs_epi32(_mm256_load_si256((__m256i*) &input[16]), _mm256_load_si256((__m256i*) &input[24]));
    __m256i result = _mm256_permutevar8x32_epi32(_mm256_max_epi8(_mm256_packs_epi16(in0, in1), zero), control);
    _mm256_store_si256((__m256i*) &output[0], result);
}

#endif //UTTT2_CLIPPED_RELU_H
