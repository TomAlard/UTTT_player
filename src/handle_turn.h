#ifndef UTTT2_HANDLE_TURN_H
#define UTTT2_HANDLE_TURN_H

#include "mcts/find_next_move.h"

typedef struct HandleTurnResult {
    Square move;
    int newRootIndex;
    int amountOfSimulations;
} HandleTurnResult;

HandleTurnResult handleTurn(Board* board, int rootIndex, double allocatedTime, Square enemyMove);

#endif //UTTT2_HANDLE_TURN_H
