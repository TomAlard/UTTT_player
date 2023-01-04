// #pragma GCC optimize("Ofast")
// #pragma GCC target("avx", "fma")


#include <string.h>
#include "forward.h"
#include "parameters.h"

#define HIDDEN_NEURONS 256


void addHiddenWeights(int i, float* restrict output) {
    for (int j = 0; j < HIDDEN_NEURONS; j++) {
        output[j] += hiddenWeights[i][j];
    }
}


void boardToInput(Board* board, float* restrict output) {
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
    addHiddenWeights(board->state.currentBoard + 180, output);
}


#define NEGATIVE_SLOPE 0.01f
void applyLeakyReLU(float* restrict input) {
    for (int i = 0; i < HIDDEN_NEURONS; i++) {
        input[i] = input[i] < 0? input[i] * NEGATIVE_SLOPE : input[i];
    }
}


float multiplyOutputWeights(const float* restrict input) {
    float result = 0.0f;
    for (int i = 0; i < HIDDEN_NEURONS; i++) {
        result += input[i] * outputWeights[i];
    }
    return result;
}


float neuralNetworkEval(Board* board) {
    float input[HIDDEN_NEURONS];
    memcpy(input, hiddenBiases, HIDDEN_NEURONS * sizeof(float));
    boardToInput(board, input);
    applyLeakyReLU(input);
    float x = multiplyOutputWeights(input) + outputBias + 0.5f;
    return x < 0? 0 : x > 1? 1 : x;
}
