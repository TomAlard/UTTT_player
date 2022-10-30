#ifndef UTTT2_ROLLOUT_H
#define UTTT2_ROLLOUT_H

#include "board.h"
#include "smart_rollout.h"

void makeRandomTemporaryMove(Board* board, RNG* rng);

Winner rollout(Board* board, RNG* rng, int gameId);

#endif //UTTT2_ROLLOUT_H
