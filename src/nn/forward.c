#pragma GCC optimize("Ofast", "unroll-loops", "omit-frame-pointer")
#pragma GCC option("arch=native", "tune=native", "no-zero-upper")
#pragma GCC target("avx2", "fma")


#include <stdalign.h>
#include "forward.h"
#include "clipped_relu.h"
#include "linear.h"

#define HIDDEN_NEURONS 256


void addHiddenWeights(int i, int16_t* restrict output) {
    for (int j = 0; j < HIDDEN_NEURONS; j++) {
        output[j] = (int16_t) (output[j] + hiddenWeights[i][j]);
    }
}


void boardToInput(Board* board, int16_t* restrict output) {
    __m256i regs[16];
    for (int i = 0; i < 16; i++) {
        regs[i] = _mm256_load_si256((__m256i*) &hiddenBiases[i * 16]);
    }

    PlayerBitBoard* p1 = &board->state.player1;
    PlayerBitBoard* currentPlayer = p1 + board->state.currentPlayer;
    PlayerBitBoard* otherPlayer = p1 + !board->state.currentPlayer;
    uint16_t bigBoard = currentPlayer->bigBoard;
    while (bigBoard) {
        addFeature(__builtin_ffs(bigBoard) - 1, regs);
        bigBoard &= bigBoard - 1;
    }
    for (int i = 0; i < 9; i++) {
        uint16_t smallBoard = currentPlayer->smallBoards[i];
        while (smallBoard) {
            addFeature(__builtin_ffs(smallBoard) + 8 + 9 * i, regs);
            smallBoard &= smallBoard - 1;
        }
    }

    bigBoard = otherPlayer->bigBoard;
    while (bigBoard) {
        addFeature(__builtin_ffs(bigBoard) + 89, regs);
        bigBoard &= bigBoard - 1;
    }
    for (int i = 0; i < 9; i++) {
        uint16_t smallBoard = otherPlayer->smallBoards[i];
        while (smallBoard) {
            addFeature(__builtin_ffs(smallBoard) + 98 + 9 * i, regs);
            smallBoard &= smallBoard - 1;
        }
    }

    for (int i = 0; i < 16; i++) {
        _mm256_store_si256((__m256i*) &output[i * 16], regs[i]);
    }
}


float neuralNetworkEvalFromHidden(int16_t* restrict input) {
    alignas(32) int8_t output[HIDDEN_NEURONS];
    applyClippedReLU256(input, output);
    float x = applyLinear256_1(output) + outputBias + 0.5f;
    return x < 0? 0 : x > 1? 1 : x;
}


float neuralNetworkEval(Board* board) {
    alignas(32) int16_t input[HIDDEN_NEURONS];
    boardToInput(board, input);
    addHiddenWeights(board->state.currentBoard + 180, input);
    return neuralNetworkEvalFromHidden(input);
}
