#ifndef UTTT2_PRIORS_H
#define UTTT2_PRIORS_H

#include <stdint.h>
#include <stdbool.h>
#include "mcts_node.h"

void initializePriorsLookupTable();

bool* getPairPriors(uint16_t smallBoard, uint16_t otherPlayerSmallBoard);

void applyPriors(Board* board, MCTSNode* parent);

#endif //UTTT2_PRIORS_H
