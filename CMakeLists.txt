cmake_minimum_required(VERSION 3.21)
project(UTTT2 C)

set(CMAKE_C_STANDARD 17)

add_executable(UTTT2 src/main.c src/bitboard.c src/bitboard.h src/square.c src/square.h src/player.c src/player.h test/tests_main.c test/tests_main.h test/bitboard_tests.c test/bitboard_tests.h test/test_util.c test/test_util.h src/util.c src/util.h src/player_bitboard.c src/player_bitboard.h test/player_bitboard_tests.c test/player_bitboard_tests.h src/mcts_node.c src/mcts_node.h test/mcts_node_tests.c test/mcts_node_tests.h)

target_link_libraries(UTTT2 m)