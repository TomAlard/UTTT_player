#ifndef UTTTCODINGAMEC_FIND_BEST_MOVE_H
#define UTTTCODINGAMEC_FIND_BEST_MOVE_H

#include "board.h"
#include "mcts_node.h"
#include "random.h"

Square find_best_move(Board* board, MCTSNode* root, double allocated_time, pcg32_random_t* rng);

#endif //UTTTCODINGAMEC_FIND_BEST_MOVE_H
