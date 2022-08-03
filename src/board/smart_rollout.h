#ifndef UTTT2_SMART_ROLLOUT_H
#define UTTT2_SMART_ROLLOUT_H

#include <stdint.h>
#include "../misc/player.h"
#include "board.h"

typedef struct RolloutState {
    uint16_t lastBoard;
    uint16_t instantWinBoards[2];
    uint16_t instantWinSmallBoards[2];
    bool hasMoreSmallBoardsThanOpponent[2];
} RolloutState;

void initializeLookupTable();

void initializeRolloutState(RolloutState* RS);

bool hasWinningMove(Board* board, RolloutState* RS);

void updateSmallBoardState(Board* board, RolloutState* RS, uint8_t boardIndex);

void updateBigBoardState(Board* board, RolloutState* RS);

#endif //UTTT2_SMART_ROLLOUT_H
