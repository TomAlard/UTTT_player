#include <stdio.h>
#include "tests_main.h"
#include "board/board_tests.h"
#include "board/player_bitboard_tests.h"
#include "mcts/mcts_node_tests.h"
#include "mcts/find_next_move_tests.h"
#include "profile_simulations.h"
#include "board/smart_rollout_tests.h"
#include "mcts/solver_tests.h"
#include "mcts/priors_tests.h"


void runTests() {
    printf("PlayerBitBoard tests...\n");
    runPlayerBitBoardTests();
    printf("SmartRollout tests...\n");
    runSmartRolloutTests();
    printf("Board tests...\n");
    runBoardTests();
    printf("MCTSNode tests...\n");
    runMCTSNodeTests();
    printf("Solver tests...\n");
    runSolverTests();
    printf("Priors tests...\n");
    runPriorsTests();
    printf("FindNextMove tests...\n");
    runFindNextMoveTests();
    printf("Amount of simulations on second move: ");
    profileSimulations();
}