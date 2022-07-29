#ifndef UTTT2_SMART_ROLLOUT_H
#define UTTT2_SMART_ROLLOUT_H

#include <stdint.h>
#include "player.h"

typedef struct RolloutState {
    uint16_t instantWinBoards[2];
    uint16_t instantWinSmallBoards[2];
} RolloutState;

void initializeLookupTable();

void initializeRolloutState(RolloutState* RS);

bool hasWinningMove(RolloutState* RS, uint8_t currentBoard, Player player);

void updateSmallBoardState(RolloutState* RS, uint8_t boardIndex, uint16_t player1SmallBoard, uint16_t player2SmallBoard);

void updateBigBoardState(RolloutState* RS, uint16_t player1BigBoard, uint16_t player2BigBoard);

#endif //UTTT2_SMART_ROLLOUT_H
