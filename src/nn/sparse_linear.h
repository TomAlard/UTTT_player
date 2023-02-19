#ifndef UTTT2_SPARSE_LINEAR_H
#define UTTT2_SPARSE_LINEAR_H

#include <stdint.h>
#include <immintrin.h>
#include <stdalign.h>
#include "parameters.h"
#include "lookup_table_indices.h"


#define NUM_INPUTS 512


inline __attribute__((always_inline)) void m256_process_chunk_alternative(
        __m256i* acc0, __m256i* acc1, __m256i* acc2, __m256i* acc3,
        __m256i  col0, __m256i  col1, __m256i  col2, __m256i  col3,
        __m256i  mul0,                __m256i  mul2
) {
    __m256i ones = _mm256_set1_epi16(1);

    __m256i prod0 = _mm256_maddubs_epi16(mul0, _mm256_unpacklo_epi8(col0, col1));
    __m256i prod1 = _mm256_maddubs_epi16(mul0, _mm256_unpackhi_epi8(col0, col1));
    __m256i prod2 = _mm256_maddubs_epi16(mul2, _mm256_unpacklo_epi8(col2, col3));
    __m256i prod3 = _mm256_maddubs_epi16(mul2, _mm256_unpackhi_epi8(col2, col3));
    *acc0 = _mm256_add_epi32(*acc0, _mm256_madd_epi16(ones, _mm256_unpacklo_epi16(prod0, prod2)));
    *acc1 = _mm256_add_epi32(*acc1, _mm256_madd_epi16(ones, _mm256_unpackhi_epi16(prod0, prod2)));
    *acc2 = _mm256_add_epi32(*acc2, _mm256_madd_epi16(ones, _mm256_unpacklo_epi16(prod1, prod3)));
    *acc3 = _mm256_add_epi32(*acc3, _mm256_madd_epi16(ones, _mm256_unpackhi_epi16(prod1, prod3)));
}


void applySparseLinear512_32(const int8_t* input, int32_t* output) {
    uint16_t nnz_input_indices[NUM_INPUTS + 16];
    int num_nnz_input_indices = 0;
    __m128i base = _mm_set1_epi16(0);
    __m128i increment = _mm_set1_epi16(8);
    for (int i = 0; i < NUM_INPUTS; i += 32) {
        __m256i input_chunk = _mm256_load_si256((__m256i*)(input + i));
        unsigned nnz = _mm256_movemask_epi8(_mm256_cmpgt_epi8(input_chunk, _mm256_setzero_si256()));

        unsigned b0 = (nnz) & 0xFF;
        unsigned b1 = (nnz >> 8) & 0xFF;
        unsigned b2 = (nnz >> 16) & 0xFF;
        unsigned b3 = (nnz >> 24) & 0xFF;

        int c0 = __builtin_popcount(b0);
        int c1 = __builtin_popcount(b1);
        int c2 = __builtin_popcount(b2);
        int c3 = __builtin_popcount(b3);

        _mm_storeu_si128(
                (__m128i*)(nnz_input_indices + num_nnz_input_indices),
                _mm_add_epi32(_mm_loadu_si128((__m128i*)(&LookupTableIndices[b0])), base)
        );
        num_nnz_input_indices += c0;
        base = _mm_add_epi32(base, increment);

        _mm_storeu_si128(
                (__m128i*)(nnz_input_indices + num_nnz_input_indices),
                _mm_add_epi32(_mm_loadu_si128((__m128i*)(&LookupTableIndices[b1])), base)
        );
        num_nnz_input_indices += c1;
        base = _mm_add_epi32(base, increment);

        _mm_storeu_si128(
                (__m128i*)(nnz_input_indices + num_nnz_input_indices),
                _mm_add_epi32(_mm_loadu_si128((__m128i*)(&LookupTableIndices[b2])), base)
        );
        num_nnz_input_indices += c2;
        base = _mm_add_epi32(base, increment);

        _mm_storeu_si128(
                (__m128i*)(nnz_input_indices + num_nnz_input_indices),
                _mm_add_epi32(_mm_loadu_si128((__m128i*)(&LookupTableIndices[b3])), base)
        );
        num_nnz_input_indices += c3;
        base = _mm_add_epi32(base, increment);
    }

    while (num_nnz_input_indices % 4 != 0)
        nnz_input_indices[num_nnz_input_indices++] = NUM_INPUTS;

    __m256i acc[4];
    for (int k = 0; k < 4; k++)
        acc[k] = _mm256_setzero_si256();

    for (int j = 0; j < num_nnz_input_indices; j += 4) {
        __m256i  mul0 = _mm256_set1_epi16((int16_t) (input[nnz_input_indices[j+0]] | (input[nnz_input_indices[j+1]] << 8)));
        __m256i  mul2 = _mm256_set1_epi16((int16_t) (input[nnz_input_indices[j+2]] | (input[nnz_input_indices[j+3]] << 8)));
        __m256i col0 = _mm256_load_si256((__m256i*) &hidden2Weights[nnz_input_indices[j+0]]);
        __m256i col1 = _mm256_load_si256((__m256i*) &hidden2Weights[nnz_input_indices[j+1]]);
        __m256i col2 = _mm256_load_si256((__m256i*) &hidden2Weights[nnz_input_indices[j+2]]);
        __m256i col3 = _mm256_load_si256((__m256i*) &hidden2Weights[nnz_input_indices[j+3]]);
        m256_process_chunk_alternative(
                &acc[0], &acc[1], &acc[2], &acc[3],
                   col0,    col1,    col2,    col3,
                   mul0,             mul2
        );
    }
    __m128i acc00 = _mm256_extracti128_si256(acc[0], 0);
    __m128i acc01 = _mm256_extracti128_si256(acc[0], 1);
    __m128i acc10 = _mm256_extracti128_si256(acc[1], 0);
    __m128i acc11 = _mm256_extracti128_si256(acc[1], 1);
    __m128i acc20 = _mm256_extracti128_si256(acc[2], 0);
    __m128i acc21 = _mm256_extracti128_si256(acc[2], 1);
    __m128i acc30 = _mm256_extracti128_si256(acc[3], 0);
    __m128i acc31 = _mm256_extracti128_si256(acc[3], 1);

    __m256i* biases_tile = (__m256i*) hidden2Biases;
    __m256i* output_tile = (__m256i*) output;
    output_tile[0] = _mm256_srai_epi32(_mm256_add_epi32(_mm256_setr_m128i(acc00, acc10), biases_tile[0]), 6);
    output_tile[1] = _mm256_srai_epi32(_mm256_add_epi32(_mm256_setr_m128i(acc20, acc30), biases_tile[1]), 6);
    output_tile[2] = _mm256_srai_epi32(_mm256_add_epi32(_mm256_setr_m128i(acc01, acc11), biases_tile[2]), 6);
    output_tile[3] = _mm256_srai_epi32(_mm256_add_epi32(_mm256_setr_m128i(acc21, acc31), biases_tile[3]), 6);
}

#endif //UTTT2_SPARSE_LINEAR_H
