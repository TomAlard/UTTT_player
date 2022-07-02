#ifndef UTTT2_HANDLE_TURN_H
#define UTTT2_HANDLE_TURN_H

#include "find_next_move.h"

typedef struct HandleTurnResult {
    Square move;
    MCTSNode* newRoot;
} HandleTurnResult;

HandleTurnResult handleTurn(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime, Square enemyMove);

#endif //UTTT2_HANDLE_TURN_H
