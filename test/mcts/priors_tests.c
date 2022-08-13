#include <stdio.h>
#include "priors_tests.h"
#include "../../src/mcts/priors.h"
#include "../test_util.h"


void verifyArrays(const bool* restrict expected, const bool* restrict actual) {
    for (int i = 0; i < 9; i++) {
        myAssert(expected[i] == actual[i]);
    }
}


#define AMOUNT_OF_TESTS 5
void pairPriorsTests() {
    initializePriorsLookupTable();
    uint16_t inputs[AMOUNT_OF_TESTS][2] = {
            {2, 16},
            {1, 256},
            {16, 32},
            {4, 32},
            {8, 128}
    };
    bool expectedOutputs[AMOUNT_OF_TESTS][9] = {
            {1, 0, 1, 0, 0, 0, 0, 0, 0},
            {0, 1, 1, 1, 0, 0, 1, 0, 0},
            {1, 1, 1, 0, 0, 0, 1, 1, 1},
            {1, 1, 0, 0, 1, 0, 1, 0, 0},
            {1, 0, 0, 0, 1, 1, 1, 0, 0}
    };
    for (int i = 0; i < AMOUNT_OF_TESTS; i++) {
        verifyArrays(expectedOutputs[i], getPairPriors(inputs[i][0], inputs[i][1]));
    }
}


void runPriorsTests() {
    printf("\tpairPriorsTests...\n");
    pairPriorsTests();
}