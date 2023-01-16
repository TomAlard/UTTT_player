#ifndef UTTT2_MAIN_H
#define UTTT2_MAIN_H

#include <stdio.h>
#include "handle_turn.h"

Square playTurn(Board* board, MCTSNode** root, double allocatedTime, Square enemyMove, int gameId);

#endif //UTTT2_MAIN_H
