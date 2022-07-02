#include <stdio.h>
#include "board_tests.h"
#include "../src/board.h"
#include "test_util.h"
#include "../src/util.h"


void anyMoveAllowedOnEmptyBoard(Board* board) {
    Square moves[TOTAL_SMALL_SQUARES];
    myAssert(generateMoves(board, moves) == TOTAL_SMALL_SQUARES);
}


void nineOrEightMovesAllowedAfterFirstMove(Board* board) {
    Square moves[TOTAL_SMALL_SQUARES];
    int amountOfMoves = generateMoves(board, moves);
    for (int i = 0; i < amountOfMoves; i++) {
        Square move = moves[i];
        makeTemporaryMove(board, move);
        int expectedAmountOfMoves = move.board == move.position? 8 : 9;
        Square nextMoves[TOTAL_SMALL_SQUARES];
        myAssert(generateMoves(board, nextMoves) == expectedAmountOfMoves);
        revertToCheckpoint(board);
    }
}


void testSecondMoveGeneration() {
    Board* board = createBoard();
    Square firstMove = {1, 0};
    makeTemporaryMove(board, firstMove);
    Square expectedMoves[9] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}};
    Square actualMoves[TOTAL_SMALL_SQUARES];
    generateMoves(board, actualMoves);
    for (int i = 0; i < 9; i++) {
        myAssert(squaresAreEqual(expectedMoves[i], actualMoves[i]));
    }
    freeBoard(board);
}


void gameSimulation(const int amountOfMoves, const Square* playedMoves, const int* possibleMoves, Winner expectedWinner) {
    Board* board = createBoard();
    Square ignoredGeneratedMoves[TOTAL_SMALL_SQUARES];
    for (int i = 0; i < amountOfMoves; i++) {
        myAssert(generateMoves(board, ignoredGeneratedMoves) == possibleMoves[i]);
        Square move = toOurNotation(playedMoves[i]);
        makeTemporaryMove(board, move);
        myAssert(getWinner(board) == (i != amountOfMoves - 1? NONE : expectedWinner));
    }
    freeBoard(board);
}


// https://www.codingame.com/replay/296363090
void reCurseVsDaFish() {
    Square playedMoves[49] = {
            {4, 4}, {5, 3}, {6, 2}, {1, 7}, {5, 4}, {8, 3}, {8, 0}, {8, 1}, {7, 3}, {4, 2}, {3, 7}, {1, 5}, {5, 7},
            {7, 5}, {4, 7}, {3, 4}, {0, 5}, {1, 6}, {3, 2}, {1, 8}, {3, 3}, {2, 0}, {7, 1}, {5, 5}, {7, 7}, {4, 5},
            {1, 4}, {3, 5}, {2, 3}, {4, 0}, {4, 1}, {1, 0}, {3, 0}, {2, 2}, {7, 6}, {5, 2}, {6, 6}, {0, 0}, {8, 5},
            {8, 6}, {7, 4}, {8, 7}, {8, 4}, {6, 3}, {7, 8}, {6, 4}, {6, 5}, {5, 0}, {3, 1}
    };
    int possibleMoves[49] = {
            81, 8, 9, 9, 7, 9, 8, 7, 8, 9, 9, 9, 8,
            7, 7, 6, 8, 8, 8, 7, 49, 9, 6, 4, 9, 3,
            38, 2, 35, 29, 6, 27, 5, 7, 8, 4, 7, 6, 15,
            6, 13, 12, 4, 3, 9, 5, 4, 3, 2
    };
    gameSimulation(49, playedMoves, possibleMoves, WIN_P1);
}


// https://www.codingame.com/replay/637133302
void yurkovVsJacek() {
    Square playedMoves[42] = {
            {4, 4}, {3, 3}, {0, 0}, {2, 2}, {8, 8}, {6, 7}, {0, 4}, {1, 5}, {4, 8}, {3, 7}, {0, 5}, {0, 8}, {2, 7},
            {8, 4}, {7, 3}, {4, 0}, {5, 1}, {6, 4}, {0, 3}, {2, 0}, {8, 0}, {7, 2}, {3, 8}, {1, 8}, {5, 8}, {6, 8},
            {2, 8}, {6, 6}, {0, 2}, {0, 6}, {1, 0}, {3, 0}, {2, 1}, {7, 4}, {5, 3}, {6, 2}, {1, 6}, {3, 2}, {1, 7},
            {3, 5}, {2, 6}, {8, 2}
    };
    int possibleMoves[42] = {
            81, 8, 9, 8, 9, 8, 9, 8, 9, 8, 7, 9, 8,
            9, 8, 9, 8, 7, 6, 7, 9, 8, 7, 7, 6, 7,
            6, 6, 6, 5, 5, 7, 4, 6, 7, 7, 4, 6, 3,
            6, 2, 6
    };
    gameSimulation(42, playedMoves, possibleMoves, WIN_P2);
}


void runBoardTests() {
    Board* board = createBoard();
    printf("\tanyMoveAllowedOnEmptyBoard...\n");
    anyMoveAllowedOnEmptyBoard(board);
    printf("\tnineOrEightMovesAllowedAfterFirstMove...\n");
    nineOrEightMovesAllowedAfterFirstMove(board);
    printf("\ttestSecondMoveGeneration...\n");
    testSecondMoveGeneration();
    printf("\treCurseVsDaFish...\n");
    reCurseVsDaFish();
    printf("\tyurkovVsJacek...\n");
    yurkovVsJacek();
    freeBoard(board);
}