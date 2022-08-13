#include <assert.h>
#include "priors.h"
#include "../board/player_bitboard.h"


bool pairPriors[512][512][9];
void calculatePairPriors(uint16_t smallBoard, uint16_t otherPlayerSmallBoard, bool* result) {
    uint16_t linesOf2Masks[24] = {
            3, 6, 24, 48, 192, 384, 5, 40, 320,  // horizontal
            9, 18, 36, 72, 144, 288, 65, 130, 260,  // vertical
            17, 272, 20, 80, 257, 68  // diagonal
    };
    for (uint8_t position = 0; position < 9; position++) {
        result[position] = false;
        if ((1 << position) == smallBoard || (1 << position) == otherPlayerSmallBoard) {
            continue;
        }
        for (int i = 0; i < 24; i++) {
            uint16_t mask = linesOf2Masks[i];
            if ((smallBoard | (1 << position)) == mask && !isWin(mask | otherPlayerSmallBoard)) {
                result[position] = true;
                break;
            }
        }
    }
}


void initializePriorsLookupTable() {
    for (uint8_t position1 = 0; position1 < 9; position1++) {
        for (uint8_t position2 = 0; position2 < 9; position2++) {
            if (position1 != position2) {
                uint16_t smallBoard = 1 << position1;
                uint16_t otherPlayerSmallBoard = 1 << position2;
                calculatePairPriors(smallBoard, otherPlayerSmallBoard, pairPriors[smallBoard][otherPlayerSmallBoard]);
            }
        }
    }
}


bool* getPairPriors(uint16_t smallBoard, uint16_t otherPlayerSmallBoard) {
    assert(ONE_BIT_SET(smallBoard) && ONE_BIT_SET(otherPlayerSmallBoard) && smallBoard != otherPlayerSmallBoard);
    return pairPriors[smallBoard][otherPlayerSmallBoard];
}