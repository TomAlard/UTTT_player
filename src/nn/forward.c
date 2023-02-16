#include <stdalign.h>
#include "forward.h"
#include "clipped_relu.h"
#include "linear.h"

#define HIDDEN2_NEURONS 32
#define HIDDEN3_NEURONS 32


void addAccumulatorWeights(int i, int16_t* restrict output) {
    for (int j = 0; j < HIDDEN1_NEURONS; j++) {
        output[j] = (int16_t)(output[j] + hidden1Weights[i][j]);
    }
}


void updateAccumulator(const int16_t* restrict input, int numNewFeatures, const int newFeatures[numNewFeatures],
                       int16_t* restrict output) {
    for (int i = 0; i < 2; i++) {
        __m256i regs[16];
        for (int j = 0; j < 16; j++) {
            regs[j] = _mm256_load_si256((__m256i*) &input[i*256 + j*16]);
        }

        for (int a = 0; a < numNewFeatures; a++) {
            int f = newFeatures[a];
            for (int j = 0; j < 16; j++) {
                regs[j] = _mm256_add_epi16(regs[j], _mm256_load_si256((__m256i*) &hidden1Weights[f][i*256 + j*16]));
            }
        }

        for (int j = 0; j < 16; j++) {
            _mm256_store_si256((__m256i*) &output[i*256 + j*16], regs[j]);
        }
    }
}


void boardToInput(Board* board, int16_t* restrict output) {
    int numActiveFeatures = 0;
    int activeFeatures[81 + 9*2 + 1];
    PlayerBitBoard* p1 = &board->state.player1;
    PlayerBitBoard* currentPlayer = p1 + board->state.currentPlayer;
    PlayerBitBoard* otherPlayer = p1 + !board->state.currentPlayer;
    uint16_t bigBoard = currentPlayer->bigBoard;
    while (bigBoard) {
        activeFeatures[numActiveFeatures++] = __builtin_ffs(bigBoard) - 1;
        bigBoard &= bigBoard - 1;
    }
    for (int i = 0; i < 9; i++) {
        uint16_t smallBoard = currentPlayer->smallBoards[i];
        while (smallBoard) {
            activeFeatures[numActiveFeatures++] = __builtin_ffs(smallBoard) + 8 + 9 * i;
            smallBoard &= smallBoard - 1;
        }
    }

    bigBoard = otherPlayer->bigBoard;
    while (bigBoard) {
        activeFeatures[numActiveFeatures++] = __builtin_ffs(bigBoard) + 89;
        bigBoard &= bigBoard - 1;
    }
    for (int i = 0; i < 9; i++) {
        uint16_t smallBoard = otherPlayer->smallBoards[i];
        while (smallBoard) {
            activeFeatures[numActiveFeatures++] = __builtin_ffs(smallBoard) + 98 + 9 * i;
            smallBoard &= smallBoard - 1;
        }
    }
    updateAccumulator(hidden1Biases, numActiveFeatures, activeFeatures, output);
}


float neuralNetworkEvalFromAccumulator(const int16_t* restrict input) {
    alignas(32) int8_t afterReLU1[HIDDEN1_NEURONS];
    applyClippedReLU512(input, afterReLU1);

    alignas(32) int32_t afterHidden1[HIDDEN2_NEURONS];
    alignas(32) int8_t afterReLU2[HIDDEN2_NEURONS];
    applyLinear512_32(afterReLU1, afterHidden1);
    applyClippedReLU32(afterHidden1, afterReLU2);

    alignas(32) int32_t afterHidden2[HIDDEN3_NEURONS];
    alignas(32) int8_t afterReLU3[HIDDEN3_NEURONS];
    applyLinear32_32(afterReLU2, afterHidden2);
    applyClippedReLU32(afterHidden2, afterReLU3);

    float x = applyLinear32_1(afterReLU3) + hidden4Bias + 0.5f;
    return x < 0? 0 : x > 1? 1 : x;
}


float neuralNetworkEval(Board* board) {
    alignas(32) int16_t input[HIDDEN1_NEURONS];
    boardToInput(board, input);
    addAccumulatorWeights(board->state.currentBoard + 180, input);
    return neuralNetworkEvalFromAccumulator(input);
}
