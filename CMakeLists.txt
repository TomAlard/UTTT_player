cmake_minimum_required(VERSION 3.21)
project(UTTT2 C)

set(CMAKE_C_STANDARD 17)
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
add_executable(UTTT2 src/main.c src/board/board.c src/board/board.h src/board/square.c src/board/square.h src/misc/player.h test/tests_main.c test/tests_main.h test/board/board_tests.c test/board/board_tests.h test/test_util.c test/test_util.h src/misc/util.c src/misc/util.h src/board/player_bitboard.c src/board/player_bitboard.h test/board/player_bitboard_tests.c test/board/player_bitboard_tests.h src/mcts/mcts_node.c src/mcts/mcts_node.h test/mcts/mcts_node_tests.c test/mcts/mcts_node_tests.h src/mcts/find_next_move.c src/mcts/find_next_move.h src/misc/random.c src/misc/random.h test/mcts/find_next_move_tests.c test/mcts/find_next_move_tests.h src/handle_turn.c src/handle_turn.h test/profile_simulations.c test/profile_simulations.h src/arena/arena.c src/main.h src/arena/arena_opponent.c src/arena/arena_opponent.h src/arena/arena_opponent.h src/arena/arena.h src/board/smart_rollout.c src/board/smart_rollout.h test/board/smart_rollout_tests.c test/board/smart_rollout_tests.h)

target_link_libraries(UTTT2 m)