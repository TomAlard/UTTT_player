#ifndef UTTT2_PARAMETERS_H
#define UTTT2_PARAMETERS_H

#include <stdint.h>

extern int16_t hidden1Weights[190][256];

extern int16_t hidden1Biases[256];

extern int8_t hidden2Weights[32][256];

extern int32_t hidden2Biases[32];

extern int8_t hidden3Weights[32][32];

extern int32_t hidden3Biases[32];

extern int8_t hidden4Weights[32];

extern float hidden4Bias;

#endif //UTTT2_PARAMETERS_H
