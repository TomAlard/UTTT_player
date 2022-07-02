cmake_minimum_required(VERSION 3.21)
project(UTTT2 C)

set(CMAKE_C_STANDARD 17)

add_executable(UTTT2 src/main.c src/board.c src/board.h src/square.c src/square.h src/player.c src/player.h test/tests_main.c test/tests_main.h test/board_tests.c test/board_tests.h test/test_util.c test/test_util.h src/util.c src/util.h src/player_bitboard.c src/player_bitboard.h test/player_bitboard_tests.c test/player_bitboard_tests.h src/mcts_node.c src/mcts_node.h test/mcts_node_tests.c test/mcts_node_tests.h src/find_next_move.c src/find_next_move.h src/random.c src/random.h test/find_next_move_tests.c test/find_next_move_tests.h src/handle_turn.c src/handle_turn.h)

target_link_libraries(UTTT2 m)