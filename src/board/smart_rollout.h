#ifndef UTTT2_SMART_ROLLOUT_H
#define UTTT2_SMART_ROLLOUT_H

#include <stdint.h>
#include "../misc/player.h"
#include "board.h"

void initializeLookupTable();

bool hasWinningMove(Board* board, uint16_t smallBoardsWithWinningMove);

void updateSmallBoardState(Board* board, uint8_t boardIndex);

void updateBigBoardState(Board* board);

#endif //UTTT2_SMART_ROLLOUT_H
