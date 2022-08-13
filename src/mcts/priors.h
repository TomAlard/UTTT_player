#ifndef UTTT2_PRIORS_H
#define UTTT2_PRIORS_H

#include <stdint.h>
#include <stdbool.h>

#define PAIR_PRIORS_BONUS 2.0f

void initializePriorsLookupTable();

bool* getPairPriors(uint16_t smallBoard, uint16_t otherPlayerSmallBoard);

#endif //UTTT2_PRIORS_H
