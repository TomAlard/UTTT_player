#include "bitboard_tests.h"
#include "../src/bitboard.h"
#include "test_util.h"
#include <stdio.h>


void boardInitiallyEmpty(BitBoard* bitBoard) {
    for (int board = 0; board < 9; board++) {
        for (int position = 0; position < 9; position++) {
            Square square = createSquare(board, position);
            myAssert(getSquare(bitBoard, square) == NONE);
        }
    }
}


void getAfterSetReturnsOriginalPlayer(BitBoard* bitBoard) {
    for (int board = 0; board < 9; board++) {
        for (int position = 0; position < 9; position++) {
            Square square = createSquare(board, position);
            myAssert(getSquare(bitBoard, square) == NONE);
            setSquare(bitBoard, square, PLAYER1);
            myAssert(getSquare(bitBoard, square) == PLAYER1);
            setSquare(bitBoard, square, NONE);
            setSquare(bitBoard, square, PLAYER2);
            myAssert(getSquare(bitBoard, square) == PLAYER2);
            setSquare(bitBoard, square, NONE);
            myAssert(getSquare(bitBoard, square) == NONE);
        }
    }
    clearBoard(bitBoard);
}


void fillBoardWithOnePlayer(BitBoard* bitBoard) {
    for (int board = 0; board < 9; board++) {
        for (int position = 0; position < 9; position++) {
            Square square = createSquare(board, position);
            setSquare(bitBoard, square, PLAYER1);
        }
    }
    for (int board = 0; board < 9; board++) {
        for (int position = 0; position < 9; position++) {
            Square square = createSquare(board, position);
            myAssert(getSquare(bitBoard, square) == PLAYER1);
        }
    }
    clearBoard(bitBoard);
}


void winSmallBoardUpdatesBigBoard(BitBoard* bitBoard) {
    int directWins[8][3] = {
            {0, 1, 2}, {3, 4, 5}, {6, 7, 8},  // horizontal
            {0, 3, 6}, {1, 4, 7}, {2, 5, 8},  // vertical
            {0, 4, 8}, {2, 4, 6},  // diagonal
    };
    int oneAdditionalMove[8][4] = {
            {0, 4, 1, 2}, {3, 4, 8, 5}, {0, 6, 7, 8},  // horizontal
            {0, 1, 3, 6}, {1, 4, 5, 7}, {6, 2, 5, 8},  // vertical
            {0, 2, 4, 8}, {2, 4, 7, 6}  // diagonal
    };
    int twoAdditionalMoves[8][5] = {
            {3, 6, 1, 2, 0}, {3, 4, 7, 8, 5}, {0, 4, 6, 7, 8},  // horizontal
            {3, 6, 4, 8, 0}, {3, 1, 4, 8, 7}, {2, 5, 0, 3, 8},  // vertical
            {0, 6, 4, 5, 8}, {4, 5, 6, 8, 2}  // diagonal
    };
    for (int board = 0; board < 9; board++) {
        for (int testCaseIndex = 0; testCaseIndex < 8; testCaseIndex++) {
            for (int i = 0; i < 3; i++) {
                Square square = createSquare(board, directWins[testCaseIndex][i]);
                myAssert(getBigBoardSquare(bitBoard, board) == NONE);
                setSquare(bitBoard, square, PLAYER1);
            }
            myAssert(getBigBoardSquare(bitBoard, board) == PLAYER1);
            clearBoard(bitBoard);
            for (int i = 0; i < 4; i++) {
                Square square = createSquare(board, oneAdditionalMove[testCaseIndex][i]);
                myAssert(getBigBoardSquare(bitBoard, board) == NONE);
                setSquare(bitBoard, square, PLAYER1);
            }
            myAssert(getBigBoardSquare(bitBoard, board) == PLAYER1);
            clearBoard(bitBoard);
            for (int i = 0; i < 5; i++) {
                Square square = createSquare(board, twoAdditionalMoves[testCaseIndex][i]);
                myAssert(getBigBoardSquare(bitBoard, board) == NONE);
                setSquare(bitBoard, square, PLAYER1);
            }
            myAssert(getBigBoardSquare(bitBoard, board) == PLAYER1);
            clearBoard(bitBoard);
        }
    }
}


void drawSmallBoardUpdatesBothBigBoards(BitBoard* bitBoard) {
    int movesLeadingToDraw[9] = {
            0, 4,
            6, 3,
            5, 1,
            7, 8,
            2
    };
    for (uint8_t board = 0; board < 9; board++) {
        Player player = PLAYER1;
        for (int i = 0; i < 9; i++) {
            Square square = createSquare(board, movesLeadingToDraw[i]);
            setSquare(bitBoard, square, player);
            player = otherPlayer(player);
        }
        myAssert(getBigBoardSquare(bitBoard, board) == BOTH);
        clearBoard(bitBoard);
    }
}


void getAndSetSquareTests() {
    BitBoard* bitBoard = createBitBoard();
    printf("boardInitiallyEmpty...\n");
    boardInitiallyEmpty(bitBoard);
    printf("getAfterSetReturnsOriginalPlayer...\n");
    getAfterSetReturnsOriginalPlayer(bitBoard);
    printf("fillBoardWithOnePlayer...\n");
    fillBoardWithOnePlayer(bitBoard);
    printf("winSmallBoardUpdatesBigBoard...\n");
    winSmallBoardUpdatesBigBoard(bitBoard);
    printf("drawSmallBoardUpdatesBothBigBoards...\n");
    drawSmallBoardUpdatesBothBigBoards(bitBoard);
    freeBitBoard(bitBoard);
}


void runBitBoardTests() {
    printf("getAndSetSquare Tests...\n");
    getAndSetSquareTests();
}