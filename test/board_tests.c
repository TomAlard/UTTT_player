#include <stdio.h>
#include "board_tests.h"
#include "../src/board.h"
#include "test_util.h"
#include "../src/util.h"


void anyMoveAllowedOnEmptyBoard() {
    Board* board = createBoard();
    Square movesArray[TOTAL_SMALL_SQUARES];
    int8_t amountOfMoves;
    generateMoves(board, movesArray, &amountOfMoves);
    myAssert(amountOfMoves == TOTAL_SMALL_SQUARES);
    freeBoard(board);
}


void nineOrEightMovesAllowedAfterFirstMove() {
    Board* board = createBoard();
    Square movesArrayFirstMove[TOTAL_SMALL_SQUARES];
    int8_t amountOfMovesFirstMove;
    Square* moves = generateMoves(board, movesArrayFirstMove, &amountOfMovesFirstMove);
    for (int i = 0; i < amountOfMovesFirstMove; i++) {
        Square move = moves[i];
        makeTemporaryMove(board, move);
        int expectedAmountOfMoves = move.board == move.position? 8 : 9;
        Square movesArray[TOTAL_SMALL_SQUARES];
        int8_t amountOfMoves;
        generateMoves(board, movesArray, &amountOfMoves);
        myAssert(amountOfMoves == expectedAmountOfMoves);
        revertToCheckpoint(board);
    }
    freeBoard(board);
}


void testSecondMoveGeneration() {
    Board* board = createBoard();
    Square firstMove = {1, 0};
    makeTemporaryMove(board, firstMove);
    Square expectedMoves[9] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}};
    Square movesArray[TOTAL_SMALL_SQUARES];
    int8_t amountOfMoves;
    Square* actualMoves = generateMoves(board, movesArray, &amountOfMoves);
    for (int i = 0; i < 9; i++) {
        myAssert(squaresAreEqual(expectedMoves[i], actualMoves[i]));
    }
    freeBoard(board);
}


void gameSimulation(int totalAmountOfMoves, Square* playedMoves, const int* possibleMoves, Winner expectedWinner) {
    Board* board = createBoard();
    Square movesArray[TOTAL_SMALL_SQUARES];
    for (int i = 0; i < totalAmountOfMoves; i++) {
        int8_t amountOfMoves;
        generateMoves(board, movesArray, &amountOfMoves);
        myAssert(amountOfMoves == possibleMoves[i]);
        Square move = toOurNotation(playedMoves[i]);
        makeTemporaryMove(board, move);
        myAssert(getWinner(board) == (i != totalAmountOfMoves - 1? NONE : expectedWinner));
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


void makeRandomTemporaryMoveMakesSameMoveAsOldGenerateMovesWay() {
    Board* board = createBoard();
    pcg32_random_t rng1;
    pcg32_srandom_r(&rng1, 69, 420);
    pcg32_random_t rng2;
    pcg32_srandom_r(&rng2, 69, 420);
    while (getWinner(board) == NONE) {
        makeRandomTemporaryMove(board, &rng1);
        Square movesArray1[TOTAL_SMALL_SQUARES];
        int8_t amountOfMoves1;
        Square* validMoves1 = generateMoves(board, movesArray1, &amountOfMoves1);
        for (int i = 0; i < amountOfMoves1; i++) {
            movesArray1[i] = validMoves1[i];
        }
        revertToCheckpoint(board);
        Square movesArray2[TOTAL_SMALL_SQUARES];
        int8_t amountOfMoves2;
        Square* validMoves2 = generateMoves(board, movesArray2, &amountOfMoves2);
        Square randomMove = validMoves2[pcg32_boundedrand_r(&rng2, amountOfMoves2)];
        makePermanentMove(board, randomMove);
        Square movesArray3[TOTAL_SMALL_SQUARES];
        int8_t amountOfMoves3;
        Square* validMoves3 = generateMoves(board, movesArray3, &amountOfMoves3);
        myAssert(amountOfMoves1 == amountOfMoves3);
        for (int i = 0; i < amountOfMoves1; i++) {
            myAssert(squaresAreEqual(movesArray1[i], validMoves3[i]));
        }
    }
    freeBoard(board);
}


void runBoardTests() {
    Board* board = createBoard();
    printf("\tanyMoveAllowedOnEmptyBoard...\n");
    anyMoveAllowedOnEmptyBoard();
    printf("\tnineOrEightMovesAllowedAfterFirstMove...\n");
    nineOrEightMovesAllowedAfterFirstMove();
    printf("\ttestSecondMoveGeneration...\n");
    testSecondMoveGeneration();
    printf("\treCurseVsDaFish...\n");
    reCurseVsDaFish();
    printf("\tyurkovVsJacek...\n");
    yurkovVsJacek();
    printf("\tmakeRandomTemporaryMoveMakesSameMoveAsOldGenerateMovesWay...\n");
    makeRandomTemporaryMoveMakesSameMoveAsOldGenerateMovesWay();
    freeBoard(board);
}