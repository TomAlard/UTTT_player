#ifndef UTTT2_FORWARD_H
#define UTTT2_FORWARD_H

#include "../board/board.h"

void addHiddenWeights(int i, int16_t* restrict output);

void setHidden(Board* board, int16_t* restrict input);

float neuralNetworkEvalFromHidden(int16_t* restrict input);

float neuralNetworkEval(Board* board);

#endif //UTTT2_FORWARD_H
