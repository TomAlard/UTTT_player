#ifndef UTTT2_ARENA_OPPONENT_H
#define UTTT2_ARENA_OPPONENT_H

#include <stdio.h>
#include "../board/square.h"

typedef struct StateOpponent StateOpponent;

StateOpponent* initializeStateOpponent();

void freeStateOpponent(StateOpponent* stateOpponent);

Square playTurnOpponent(StateOpponent** state, double allocatedTime, Square enemyMove);

#endif //UTTT2_ARENA_OPPONENT_H
