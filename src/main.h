#ifndef UTTTCODINGAMEC_MAIN_H
#define UTTTCODINGAMEC_MAIN_H

#include <stdio.h>
#include "board.h"
#include "mcts_node.h"
#include "random.h"

Square play_best_move(int enemy_row, int enemy_col, Board* board, MCTSNode** root, pcg32_random_t* rng, double time,
                      bool first);

void play_game(FILE* file, double time);

#endif //UTTTCODINGAMEC_MAIN_H
