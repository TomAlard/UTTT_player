#ifndef UTTT2_FORWARD_H
#define UTTT2_FORWARD_H

#include <immintrin.h>
#include "../board/board.h"
#include "parameters.h"

#define HIDDEN1_NEURONS 256

inline __attribute__((always_inline)) void addFeature(int feature, __m256i regs[16]) {
    for (int i = 0; i < 16; i++) {
        regs[i] = _mm256_add_epi16(regs[i], _mm256_load_si256((__m256i*) &hidden1Weights[feature][i * 16]));
    }
}

void updateAccumulator(const int16_t* restrict input, int numNewFeatures, const int newFeatures[numNewFeatures],
                       int16_t* restrict output);

void boardToInput(Board* board, int16_t* restrict output);

float neuralNetworkEvalFromAccumulator(const int16_t* restrict input);

float neuralNetworkEval(Board* board);

#endif //UTTT2_FORWARD_H
