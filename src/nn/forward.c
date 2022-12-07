#pragma GCC optimize("Ofast")
#pragma GCC target("avx2", "fma")


#include "forward.h"
#include "parameters.h"
#include "../misc/util.h"


void boardToInput(Board* board, float* restrict result) {
    int add = board->state.currentPlayer == PLAYER1? 0 : 9;
    uint8_t currentBoard = board->state.currentBoard;
    for (int i = 0; i < 9; i++) {
        bool p1Set = BIT_CHECK(board->state.player1.bigBoard, i);
        bool p2Set = BIT_CHECK(board->state.player2.bigBoard, i);
        if (p1Set || p2Set) {
            int value = p1Set && p2Set ? 8 : p1Set ? 6 : 7;
            for (int j = 0; j < 9; j++) {
                result[162*i + 18*j + value+add] = 1.0f;
            }
        } else {
            for (int j = 0; j < 9; j++) {
                int value = BIT_CHECK(board->state.player1.smallBoards[i], j)? 1
                          : BIT_CHECK(board->state.player2.smallBoards[i], j)? 2
                          : 0;
                if (i == currentBoard || currentBoard == 9) {
                    value += 3;
                }
                result[162*i + 18*j + value+add] = 1.0f;
            }
        }
    }
}


void multiplyHidden1Weights(const float* restrict input, float* restrict output) {
    for (int i = 0; i < HIDDEN_1_NEURONS; i++) {
        for (int j = 0; j < INPUT_NEURONS; j++) {
            output[i] += hidden1Weights[i][j] * input[j];
        }
    }
}


void multiplyHidden2Weights(const float* restrict input, float* restrict output) {
    for (int i = 0; i < HIDDEN_2_NEURONS; i++) {
        for (int j = 0; j < HIDDEN_1_NEURONS; j++) {
            output[i] += hidden2Weights[i][j] * input[j];
        }
    }
}


void multiplyHidden3Weights(const float* restrict input, float* restrict output) {
    for (int i = 0; i < HIDDEN_3_NEURONS; i++) {
        for (int j = 0; j < HIDDEN_2_NEURONS; j++) {
            output[i] += hidden3Weights[i][j] * input[j];
        }
    }
}


void addHidden1Biases(float* restrict input) {
    for (int i = 0; i < HIDDEN_1_NEURONS; i++) {
        input[i] += hidden1Biases[i];
    }
}


void addHidden2Biases(float* restrict input) {
    for (int i = 0; i < HIDDEN_2_NEURONS; i++) {
        input[i] += hidden2Biases[i];
    }
}


void addHidden3Biases(float* restrict input) {
    for (int i = 0; i < HIDDEN_3_NEURONS; i++) {
        input[i] += hidden3Biases[i];
    }
}


#define NEGATIVE_SLOPE 0.01f
void applyLeakyReLU1(float* restrict input) {
    for (int i = 0; i < HIDDEN_1_NEURONS; i++) {
        input[i] = input[i] < 0? input[i] * NEGATIVE_SLOPE : input[i];
    }
}


void applyLeakyReLU2(float* restrict input) {
    for (int i = 0; i < HIDDEN_2_NEURONS; i++) {
        input[i] = input[i] < 0? input[i] * NEGATIVE_SLOPE : input[i];
    }
}


float multiplyOutputWeights(const float* restrict input) {
    float result = 0.0f;
    for (int i = 0; i < HIDDEN_3_NEURONS; i++) {
        result += input[i] * outputWeights[i];
    }
    return result;
}


float neuralNetworkEval(Board* board) {
    float input[INPUT_NEURONS] = {0.0f};
    boardToInput(board, input);

    float afterHidden1[HIDDEN_1_NEURONS] = {0.0f};
    multiplyHidden1Weights(input, afterHidden1);
    addHidden1Biases(afterHidden1);
    applyLeakyReLU1(afterHidden1);

    float afterHidden2[HIDDEN_2_NEURONS] = {0.0f};
    multiplyHidden2Weights(afterHidden1, afterHidden2);
    addHidden2Biases(afterHidden2);
    applyLeakyReLU2(afterHidden2);

    float afterHidden3[HIDDEN_3_NEURONS] = {0.0f};
    multiplyHidden3Weights(afterHidden2, afterHidden3);
    addHidden3Biases(afterHidden3);
    applyLeakyReLU2(afterHidden3);

    float x = multiplyOutputWeights(afterHidden3) + outputBias + 0.5f;
    return x < 0? 0 : x > 1? 1 : x;
}
