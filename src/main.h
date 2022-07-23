#ifndef UTTT2_MAIN_H
#define UTTT2_MAIN_H

#include <stdio.h>
#include "handle_turn.h"

Square playTurn(Board* board, MCTSNode** root, pcg32_random_t* rng, double allocatedTime, Square enemyMove);

#endif //UTTT2_MAIN_H
