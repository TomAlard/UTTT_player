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

void updateSmallBoardState(RolloutState* RS, uint8_t boardIndex, uint16_t smallBoard, uint16_t otherSmallBoard, Player player);

void updateBigBoardState(RolloutState* RS, uint16_t bigBoard, uint16_t otherBigBoard, Player player);

#endif //UTTT2_SMART_ROLLOUT_H
