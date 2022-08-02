#ifndef UTTT2_FIND_NEXT_MOVE_H
#define UTTT2_FIND_NEXT_MOVE_H

#include "../board/board.h"
#include "mcts_node.h"
#include "../misc/random.h"

int findNextMove(Board* board, MCTSNode* root, RNG* rng, double allocatedTime);

#endif //UTTT2_FIND_NEXT_MOVE_H
