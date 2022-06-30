#include <stdio.h>
#include "bitboard_tests.h"
#include "../src/bitboard.h"
#include "test_util.h"


void anyMoveAllowedOnEmptyBoard(BitBoard* bitBoard) {
    Square moves[TOTAL_SMALL_SQUARES];
    myAssert(generateMoves(bitBoard, moves) == TOTAL_SMALL_SQUARES);
}


void nineOrEightMovesAllowedAfterFirstMove(BitBoard* bitBoard) {
    Square moves[TOTAL_SMALL_SQUARES];
    int amountOfMoves = generateMoves(bitBoard, moves);
    for (int i = 0; i < amountOfMoves; i++) {
        Square move = moves[i];
        makeMove(bitBoard, move);
        int expectedAmountOfMoves = move.board == move.position? 8 : 9;
        Square nextMoves[TOTAL_SMALL_SQUARES];
        myAssert(generateMoves(bitBoard, nextMoves) == expectedAmountOfMoves);
        revertToCheckpoint(bitBoard);
    }
}


void runBitBoardTests() {
    BitBoard* bitBoard = createBitBoard();
    printf("\tanyMoveAllowedOnEmptyBoard...\n");
    anyMoveAllowedOnEmptyBoard(bitBoard);
    printf("\tnineOrEightMovesAllowedAfterFirstMove...\n");
    nineOrEightMovesAllowedAfterFirstMove(bitBoard);
    freeBitBoard(bitBoard);
}