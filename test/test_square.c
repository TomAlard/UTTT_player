#include <stdio.h>
#include "test_square.h"
#include "../src/square.h"
#include "../src/util.h"


void test_to_our_notation() {
    Square test_cases[] = {
            {0, 0}, {0, 0},
            {4, 4}, {4, 4},
            {0, 6}, {2, 0},
            {7, 3}, {7, 3},
            {6, 8}, {8, 2},
            {2, 5}, {1, 8}
    };

    int amount_of_test_cases = ARRAY_LENGTH(test_cases);
    for (int i = 0; i < amount_of_test_cases; i += 2) {
        Square unconverted = test_cases[i];
        Square converted = to_our_notation(unconverted.grid, unconverted.pos);
        Square expected = test_cases[i+1];
        my_assert(squares_are_equal(converted, expected), "test_to_our_notation: Conversion failed");
    }
}


void test_to_game_notation() {
    Square test_cases[] = {
            {0, 0}, {0, 0},
            {4, 4}, {4, 4},
            {0, 6}, {2, 0},
            {7, 3}, {7, 3},
            {6, 8}, {8, 2},
            {2, 5}, {1, 8}
    };

    int amount_of_test_cases = ARRAY_LENGTH(test_cases);
    for (int i = 0; i < amount_of_test_cases; i += 2) {
        Square unconverted = test_cases[i+1];
        Square converted = to_game_notation(unconverted);
        Square expected = test_cases[i];
        my_assert(squares_are_equal(converted, expected), "test_to_game_notation: Conversion failed");
    }
}


void run_square_tests() {
    fprintf(stderr, "test_to_our_notation\n");
    test_to_our_notation();
    fprintf(stderr, "test_to_game_notation\n");
    test_to_game_notation();
}