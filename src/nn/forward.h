#ifndef UTTT2_FORWARD_H
#define UTTT2_FORWARD_H

#include "../board/board.h"

void addHiddenWeights(int i, float* restrict output);

void setHidden(Board* board, float* restrict input);

float neuralNetworkEvalFromHidden(float* restrict input);

float neuralNetworkEval(Board* board);

#endif //UTTT2_FORWARD_H
