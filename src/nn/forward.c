#pragma GCC optimize("Ofast")
#pragma GCC target("avx", "fma")


#include "forward.h"
#include "parameters.h"


void boardToInput(Board* board, float* restrict result) {
    PlayerBitBoard* p1 = &board->state.player1;
    PlayerBitBoard* currentPlayer = p1 + board->state.currentPlayer;
    PlayerBitBoard* otherPlayer = p1 + !board->state.currentPlayer;
    uint16_t bigBoard = currentPlayer->bigBoard;
    while (bigBoard) {
        result[__builtin_ffs(bigBoard) - 1] = 1.0f;
        bigBoard &= bigBoard - 1;
    }
    for (int i = 0; i < 9; i++) {
        uint16_t smallBoard = currentPlayer->smallBoards[i];
        while (smallBoard) {
            result[__builtin_ffs(smallBoard) + 8 + 9*i] = 1.0f;
            smallBoard &= smallBoard - 1;
        }
    }

    bigBoard = otherPlayer->bigBoard;
    while (bigBoard) {
        result[__builtin_ffs(bigBoard) + 89] = 1.0f;
        bigBoard &= bigBoard - 1;
    }
    for (int i = 0; i < 9; i++) {
        uint16_t smallBoard = otherPlayer->smallBoards[i];
        while (smallBoard) {
            result[__builtin_ffs(smallBoard) + 98 + 9*i] = 1.0f;
            smallBoard &= smallBoard - 1;
        }
    }
    result[board->state.currentBoard + 180] = 1.0f;
}


void multiplyHiddenWeights(const float* restrict input, float* restrict output) {
    for (int i = 0; i < 190; i++) {
        for (int j = 0; j < 128; j++) {
            output[j] += hiddenWeights[i][j] * input[i];
        }
    }
}


void addHiddenBiases(float* restrict input) {
    for (int i = 0; i < 128; i++) {
        input[i] += hiddenBiases[i];
    }
}


#define NEGATIVE_SLOPE 0.01f
void applyLeakyReLU(float* restrict input) {
    for (int i = 0; i < 128; i++) {
        input[i] = input[i] < 0? input[i] * NEGATIVE_SLOPE : input[i];
    }
}


float multiplyOutputWeights(const float* restrict input) {
    float result = 0.0f;
    for (int i = 0; i < 128; i++) {
        result += input[i] * outputWeights[i];
    }
    return result;
}


float neuralNetworkEval(Board* board) {
    float input[190] = {0.0f};
    boardToInput(board, input);
    float output[128] = {0.0f};
    multiplyHiddenWeights(input, output);
    addHiddenBiases(output);
    applyLeakyReLU(output);
    float x = multiplyOutputWeights(output) + outputBias + 0.5f;
    return x < 0? 0 : x > 1? 1 : x;
}
