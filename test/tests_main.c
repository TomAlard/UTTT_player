#include <stdio.h>
#include "tests_main.h"
#include "board_tests.h"
#include "player_bitboard_tests.h"
#include "mcts_node_tests.h"
#include "find_next_move_tests.h"


void runTests() {
    printf("PlayerBitBoard tests...\n");
    runPlayerBitBoardTests();
    printf("Board tests...\n");
    runBoardTests();
    printf("MCTSNode tests...\n");
    runMCTSNodeTests();
    printf("runFindNextMoveTests...\n");
    runFindNextMoveTests();
}