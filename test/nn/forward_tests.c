#include <stdio.h>
#include "../test_util.h"
#include "../../src/nn/forward.h"
#include "forward_tests.h"


void runForwardTests() {
    Board* board = createBoard();
    printf("Eval: %f\n", neuralNetworkEval(board));
    freeBoard(board);
}
