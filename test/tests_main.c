#include <stdio.h>
#include "tests_main.h"
#include "bitboard_tests.h"
#include "player_bitboard_tests.h"


void runTests() {
    printf("PlayerBitBoard tests...\n");
    runPlayerBitBoardTests();
    printf("BitBoard tests...\n");
    runBitBoardTests();
}