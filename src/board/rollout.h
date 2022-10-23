#ifndef UTTT2_ROLLOUT_H
#define UTTT2_ROLLOUT_H

#include "board.h"
#include "smart_rollout.h"

void makeRandomTemporaryMove(Board* board, RNG* rng);

float rollout(Board* board, RNG* rng, Player player);

#endif //UTTT2_ROLLOUT_H
