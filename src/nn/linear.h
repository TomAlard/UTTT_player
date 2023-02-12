#ifndef UTTT2_LINEAR_H
#define UTTT2_LINEAR_H

#include <stdint.h>
#include <immintrin.h>

#define LOG2_WEIGHT_SCALE 6


inline __attribute__((always_inline)) void m256_add_dpbusd_epi32(__m256i* acc, __m256i a, __m256i b) {
    __m256i one = _mm256_set_epi16(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    __m256i product0 = _mm256_maddubs_epi16(a, b);
    product0 = _mm256_madd_epi16(product0, one);
    *acc = _mm256_add_epi32(*acc, product0);
}


inline __attribute__((always_inline)) __m128i m256_haddx4(__m256i sum0, __m256i sum1, __m256i sum2, __m256i sum3, __m128i bias) {
    sum0 = _mm256_hadd_epi32(sum0, sum1);
    sum2 = _mm256_hadd_epi32(sum2, sum3);
    sum0 = _mm256_hadd_epi32(sum0, sum2);
    __m128i sum128lo = _mm256_castsi256_si128(sum0);
    __m128i sum128hi = _mm256_extracti128_si256(sum0, 1);
    return _mm_add_epi32(_mm_add_epi32(sum128lo, sum128hi), bias);
}


inline __attribute__((always_inline)) void applyLinear256_32(const int8_t* input, int32_t* output) {
    for (int i = 0; i < 8; i++) {
        __m256i sum0 = _mm256_setzero_si256();
        __m256i sum1 = _mm256_setzero_si256();
        __m256i sum2 = _mm256_setzero_si256();
        __m256i sum3 = _mm256_setzero_si256();
        for (int j = 0; j < 8; j++) {
            __m256i in = _mm256_load_si256((__m256i*) &input[j * 32]);
            m256_add_dpbusd_epi32(&sum0, in, _mm256_load_si256((__m256i*) &hidden2Weights[i*4 + 0][j * 32]));
            m256_add_dpbusd_epi32(&sum1, in, _mm256_load_si256((__m256i*) &hidden2Weights[i*4 + 1][j * 32]));
            m256_add_dpbusd_epi32(&sum2, in, _mm256_load_si256((__m256i*) &hidden2Weights[i*4 + 2][j * 32]));
            m256_add_dpbusd_epi32(&sum3, in, _mm256_load_si256((__m256i*) &hidden2Weights[i*4 + 3][j * 32]));
        }
        __m128i bias = _mm_load_si128((__m128i*) &hidden2Biases[i * 4]);
        __m128i outval = m256_haddx4(sum0, sum1, sum2, sum3, bias);
        outval = _mm_srai_epi32(outval, LOG2_WEIGHT_SCALE);
        _mm_store_si128((__m128i*) &output[i * 4], outval);
    }
}


inline __attribute__((always_inline)) void applyLinear32_32(const int8_t* input, int32_t* output) {
    for (int i = 0; i < 8; i++) {
        __m256i sum0 = _mm256_setzero_si256();
        __m256i sum1 = _mm256_setzero_si256();
        __m256i sum2 = _mm256_setzero_si256();
        __m256i sum3 = _mm256_setzero_si256();
        __m256i in = _mm256_load_si256((__m256i*) &input[0]);
        m256_add_dpbusd_epi32(&sum0, in, _mm256_load_si256((__m256i*) &hidden3Weights[i*4 + 0][0]));
        m256_add_dpbusd_epi32(&sum1, in, _mm256_load_si256((__m256i*) &hidden3Weights[i*4 + 1][0]));
        m256_add_dpbusd_epi32(&sum2, in, _mm256_load_si256((__m256i*) &hidden3Weights[i*4 + 2][0]));
        m256_add_dpbusd_epi32(&sum3, in, _mm256_load_si256((__m256i*) &hidden3Weights[i*4 + 3][0]));
        __m128i bias = _mm_load_si128((__m128i*) &hidden3Biases[i * 4]);
        __m128i outval = m256_haddx4(sum0, sum1, sum2, sum3, bias);
        outval = _mm_srai_epi32(outval, LOG2_WEIGHT_SCALE);
        _mm_store_si128((__m128i*) &output[i * 4], outval);
    }
}

#endif //UTTT2_LINEAR_H
