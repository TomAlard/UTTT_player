cmake_minimum_required(VERSION 3.17)
project(UTTTCodinGameC C)

set(CMAKE_C_STANDARD 99)

add_executable(UTTTCodinGameC main.c test/test_board.c test/test_board.h test/test_main.c test/test_main.h src/main.c src/board.c src/board.h src/square.c src/square.h src/util.c src/util.h src/mcts_node.c src/mcts_node.h)