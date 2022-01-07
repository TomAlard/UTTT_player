#include <stdio.h>
#include "test_main.h"
#include "test_board.h"
#include "test_square.h"
#include "integration_tests.h"


void run_tests() {
    fprintf(stderr, "=====START TESTS=====\n");
    fprintf(stderr, "=====START SQUARE TESTS=====\n");
    run_square_tests();
    fprintf(stderr, "=====START BOARD TESTS=====\n");
    run_board_tests();
    fprintf(stderr, "=====START INTEGRATION TESTS=====\n");
    run_integration_tests();
    fprintf(stderr, "=====END TESTS=====\n\n\n");
}
