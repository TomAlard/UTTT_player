#pragma GCC optimize("Ofast", "unroll-loops", "omit-frame-pointer")
#pragma GCC option("arch=native", "tune=native", "no-zero-upper")
#pragma GCC target("avx2", "fma")


#include <string.h>
#include "forward.h"
#include "parameters.h"

#define HIDDEN_NEURONS 256


void addHiddenWeights(int i, int16_t* restrict output) {
    output = __builtin_assume_aligned(output, 32);
    for (int j = 0; j < HIDDEN_NEURONS; j++) {
        output[j] = (int16_t)(output[j] + hiddenWeights[i][j]);
    }
}


void boardToInput(Board* board, int16_t* restrict output) {
    PlayerBitBoard* p1 = &board->state.player1;
    PlayerBitBoard* currentPlayer = p1 + board->state.currentPlayer;
    PlayerBitBoard* otherPlayer = p1 + !board->state.currentPlayer;
    uint16_t bigBoard = currentPlayer->bigBoard;
    while (bigBoard) {
        addHiddenWeights(__builtin_ffs(bigBoard) - 1, output);
        bigBoard &= bigBoard - 1;
    }
    for (int i = 0; i < 9; i++) {
        uint16_t smallBoard = currentPlayer->smallBoards[i];
        while (smallBoard) {
            addHiddenWeights(__builtin_ffs(smallBoard) + 8 + 9 * i, output);
            smallBoard &= smallBoard - 1;
        }
    }

    bigBoard = otherPlayer->bigBoard;
    while (bigBoard) {
        addHiddenWeights(__builtin_ffs(bigBoard) + 89, output);
        bigBoard &= bigBoard - 1;
    }
    for (int i = 0; i < 9; i++) {
        uint16_t smallBoard = otherPlayer->smallBoards[i];
        while (smallBoard) {
            addHiddenWeights(__builtin_ffs(smallBoard) + 98 + 9 * i, output);
            smallBoard &= smallBoard - 1;
        }
    }
}


void applyClippedReLU(const int16_t* restrict input, int8_t* restrict output) {
    for (int i = 0; i < HIDDEN_NEURONS; i++) {
        output[i] = (int8_t)(input[i] <= 0? 0 : input[i] >= 127? 127 : input[i]);
    }
}


float multiplyOutputWeights(const int8_t* restrict input) {
    int32_t result = 0;
    for (int i = 0; i < HIDDEN_NEURONS; i++) {
        result += input[i] * outputWeights[i];
    }
    return (float)result / (127*64);
}


void setHidden(Board* board, int16_t* restrict input) {
    memcpy(input, hiddenBiases, HIDDEN_NEURONS * sizeof(int16_t));
    boardToInput(board, input);
}


float neuralNetworkEvalFromHidden(int16_t* restrict input) {
    int8_t output[HIDDEN_NEURONS];
    applyClippedReLU(input, output);
    float x = multiplyOutputWeights(output) + outputBias + 0.5f;
    return x < 0? 0 : x > 1? 1 : x;
}


float neuralNetworkEval(Board* board) {
    int16_t input[HIDDEN_NEURONS];
    setHidden(board, input);
    addHiddenWeights(board->state.currentBoard + 180, input);
    return neuralNetworkEvalFromHidden(input);
}
