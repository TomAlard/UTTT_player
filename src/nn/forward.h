#ifndef UTTT2_FORWARD_H
#define UTTT2_FORWARD_H

#include <immintrin.h>
#include "../board/board.h"
#include "parameters.h"

#define HIDDEN1_NEURONS 512

void updateAccumulator(const int16_t* restrict input, int numNewFeatures, const int newFeatures[numNewFeatures],
                       int16_t* restrict output);

void boardToInput(Board* board, int16_t* restrict output);

float neuralNetworkEvalFromAccumulator(const int16_t* restrict input);

float neuralNetworkEval(Board* board);

#endif //UTTT2_FORWARD_H
