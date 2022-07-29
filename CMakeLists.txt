cmake_minimum_required(VERSION 3.21)
project(UTTT2 C)

set(CMAKE_C_STANDARD 17)
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
add_executable(UTTT2 src/main.c src/board.c src/board.h src/square.c src/square.h src/player.h test/tests_main.c test/tests_main.h test/board_tests.c test/board_tests.h test/test_util.c test/test_util.h src/util.c src/util.h src/player_bitboard.c src/player_bitboard.h test/player_bitboard_tests.c test/player_bitboard_tests.h src/mcts_node.c src/mcts_node.h test/mcts_node_tests.c test/mcts_node_tests.h src/find_next_move.c src/find_next_move.h src/random.c src/random.h test/find_next_move_tests.c test/find_next_move_tests.h src/handle_turn.c src/handle_turn.h test/profile_simulations.c test/profile_simulations.h src/arena.c src/main.h src/arena_opponent.c src/arena_opponent.h src/arena_opponent.h src/arena.h src/smart_rollout.c src/smart_rollout.h test/smart_rollout_tests.c test/smart_rollout_tests.h)

target_link_libraries(UTTT2 m)