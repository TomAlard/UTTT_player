#include <stdalign.h>
#include "forward.h"
#include "clipped_relu.h"
#include "linear.h"
#include "../misc/util.h"

#define HIDDEN2_NEURONS 32
#define HIDDEN3_NEURONS 32


void addAccumulatorWeights(int i, int16_t* restrict output) {
    for (int j = 0; j < HIDDEN1_NEURONS; j++) {
        output[j] = (int16_t)(output[j] + hidden1Weights[i][j]);
    }
}


void boardToInput(Board* board, int16_t* restrict output) {
    __m256i regs[16];
    for (int i = 0; i < 16; i++) {
        regs[i] = _mm256_load_si256((__m256i*) &hidden1Biases[i * 16]);
    }

    PlayerBitBoard* p1 = &board->state.player1;
    PlayerBitBoard* currentPlayer = p1 + board->state.currentPlayer;
    PlayerBitBoard* otherPlayer = p1 + !board->state.currentPlayer;
    for (int i = 0; i < 9; i++) {
        bool currentPlayerBigBoardBitSet = BIT_CHECK(currentPlayer->bigBoard, i);
        bool otherPlayerBigBoardBitSet = BIT_CHECK(otherPlayer->bigBoard, i);
        if (currentPlayerBigBoardBitSet || otherPlayerBigBoardBitSet) {
            int value = currentPlayerBigBoardBitSet && otherPlayerBigBoardBitSet ? 4
                    : currentPlayerBigBoardBitSet ? 2
                    : 3;
            for (int j = 0; j < 9; j++) {
                addFeature(regs, 45*i + 5*j + value);
            }
        } else {
            for (int j = 0; j < 9; j++) {
                if (BIT_CHECK(currentPlayer->smallBoards[i], j)) {
                    addFeature(regs, 45*i + 5*j);
                } else if (BIT_CHECK(otherPlayer->smallBoards[i], j)) {
                    addFeature(regs, 45*i + 5*j + 1);
                }
            }
        }
    }

    for (int i = 0; i < 16; i++) {
        _mm256_store_si256((__m256i*) &output[i * 16], regs[i]);
    }
}


float multiplyOutputWeights(const int8_t* restrict input) {
    int32_t result = 0;
    for (int i = 0; i < HIDDEN3_NEURONS; i++) {
        result += input[i] * hidden4Weights[i];
    }
    return (float)result / (127*64);
}


float neuralNetworkEvalFromAccumulator(const int16_t* restrict input) {
    alignas(32) int8_t afterReLU1[HIDDEN1_NEURONS];
    applyClippedReLU256(input, afterReLU1);

    alignas(32) int32_t afterHidden1[HIDDEN2_NEURONS];
    alignas(32) int8_t afterReLU2[HIDDEN2_NEURONS];
    applyLinear256_32(afterReLU1, afterHidden1);
    applyClippedReLU32(afterHidden1, afterReLU2);

    alignas(32) int32_t afterHidden2[HIDDEN3_NEURONS];
    alignas(32) int8_t afterReLU3[HIDDEN3_NEURONS];
    applyLinear32_32(afterReLU2, afterHidden2);
    applyClippedReLU32(afterHidden2, afterReLU3);

    float x = multiplyOutputWeights(afterReLU3) + hidden4Bias + 0.5f;
    return x < 0? 0 : x > 1? 1 : x;
}


float neuralNetworkEval(Board* board) {
    alignas(32) int16_t input[HIDDEN1_NEURONS];
    boardToInput(board, input);
    addAccumulatorWeights(board->state.currentBoard + 405, input);
    return neuralNetworkEvalFromAccumulator(input);
}
