#ifndef UTTT2_FORWARD_H
#define UTTT2_FORWARD_H

#include <immintrin.h>
#include "../board/board.h"
#include "parameters.h"

#define HIDDEN1_NEURONS 256

inline __attribute__((always_inline)) void addFeature(__m256i regs[16], int f) {
    for (int i = 0; i < 16; i++) {
        regs[i] = _mm256_add_epi16(regs[i], _mm256_load_si256((__m256i*) &hidden1Weights[f][i * 16]));
    }
}


inline __attribute__((always_inline)) void removeFeature(__m256i regs[16], int f) {
    for (int i = 0; i < 16; i++) {
        regs[i] = _mm256_sub_epi16(regs[i], _mm256_load_si256((__m256i*) &hidden1Weights[f][i * 16]));
    }
}


inline __attribute__((always_inline))
void setBoardValues(__m256i regs[16], PlayerBitBoard* currentPlayerBitBoard, PlayerBitBoard* otherPlayerBitBoard,
                    uint8_t board, int value) {
    uint16_t currentPlayerSmallBoard = currentPlayerBitBoard->smallBoards[board];
    uint16_t otherPlayerSmallBoard = otherPlayerBitBoard->smallBoards[board];
    while (currentPlayerSmallBoard) {
        removeFeature(regs, 45*board + 5*(__builtin_ffs(currentPlayerSmallBoard) - 1));
        currentPlayerSmallBoard &= currentPlayerSmallBoard - 1;
    }
    while (otherPlayerSmallBoard) {
        removeFeature(regs, 45*board + 5*(__builtin_ffs(otherPlayerSmallBoard) - 1) + 1);
        otherPlayerSmallBoard &= otherPlayerSmallBoard - 1;
    }

    for (int i = 0; i < 9; i++) {
        addFeature(regs, 45*board + 5*i + value);
    }
}

void boardToInput(Board* board, int16_t* restrict output);

float neuralNetworkEvalFromAccumulator(const int16_t* restrict input);

float neuralNetworkEval(Board* board);

#endif //UTTT2_FORWARD_H
