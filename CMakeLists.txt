cmake_minimum_required(VERSION 3.17)
project(UTTTCodinGameC C)

set(CMAKE_C_STANDARD 99)

add_executable(UTTTCodinGameC test/test_board.c test/test_board.h test/test_main.c test/test_main.h src/main.c src/board.c src/board.h src/square.c src/square.h src/util.c src/util.h src/mcts_node.c src/mcts_node.h src/find_best_move.c src/find_best_move.h src/random.c src/random.h test/test_square.c test/test_square.h src/main.h test/integration_tests.c test/integration_tests.h)