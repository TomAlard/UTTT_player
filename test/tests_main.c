#include <stdio.h>
#include "tests_main.h"
#include "board/board_tests.h"
#include "board/player_bitboard_tests.h"
#include "mcts/mcts_node_tests.h"
#include "mcts/find_next_move_tests.h"
#include "profile_simulations.h"
#include "nn/forward_tests.h"


void runTests() {
    printf("PlayerBitBoard tests...\n");
    runPlayerBitBoardTests();
    printf("Forward tests...\n");
    runForwardTests();
    printf("Board tests...\n");
    runBoardTests();
    printf("MCTSNode tests...\n");
    runMCTSNodeTests();
    printf("FindNextMove tests...\n");
    runFindNextMoveTests();
    printf("Amount of simulations on second move: ");
    profileSimulations();
}