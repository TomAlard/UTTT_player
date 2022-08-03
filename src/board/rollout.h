#ifndef UTTT2_ROLLOUT_H
#define UTTT2_ROLLOUT_H

#include "board.h"

void makeRandomTemporaryMove(Board* board, RolloutState* RS, RNG* rng);

Winner rollout(Board* board, RNG* rng);

#endif //UTTT2_ROLLOUT_H
