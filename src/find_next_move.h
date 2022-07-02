#ifndef UTTT2_FIND_NEXT_MOVE_H
#define UTTT2_FIND_NEXT_MOVE_H

#include "board.h"
#include "mcts_node.h"
#include "random.h"

Square findNextMove(Board* board, MCTSNode* root, double allocatedTime, pcg32_random_t* rng);

#endif //UTTT2_FIND_NEXT_MOVE_H
