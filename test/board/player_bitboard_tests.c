#include <stdio.h>
#include "player_bitboard_tests.h"
#include "../../src/board/player_bitboard.h"
#include "../test_util.h"
#include "../../src/misc/util.h"


bool squareIsOccupied(PlayerBitBoard* playerBitBoard, Square square) {
    return BIT_CHECK(playerBitBoard->smallBoards[square.board], square.position);
}


void boardInitiallyEmpty() {
    PlayerBitBoard playerBitBoard;
    initializePlayerBitBoard(&playerBitBoard);
    for (int board = 0; board < 9; board++) {
        for (int position = 0; position < 9; position++) {
            Square square = createSquare(board, position);
            myAssert(!squareIsOccupied(&playerBitBoard, square));
        }
    }
}


void testSetSquareOccupied() {
    PlayerBitBoard playerBitBoard;
    initializePlayerBitBoard(&playerBitBoard);
    PlayerBitBoard otherPlayerBitBoard;
    initializePlayerBitBoard(&otherPlayerBitBoard);
    for (int board = 0; board < 9; board++) {
        for (int position = 0; position < 9; position++) {
            Square square = createSquare(board, position);
            myAssert(!squareIsOccupied(&playerBitBoard, square));
            setSquareOccupied(&playerBitBoard, &otherPlayerBitBoard, square);
            myAssert(squareIsOccupied(&playerBitBoard, square));
        }
    }
}


bool boardIsWon(PlayerBitBoard* playerBitBoard, uint8_t board) {
    return BIT_CHECK(playerBitBoard->bigBoard, board);
}


void winSmallBoardUpdatesBigBoard() {
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
    PlayerBitBoard playerBitBoard;
    initializePlayerBitBoard(&playerBitBoard);
    PlayerBitBoard otherPlayerBitBoard;
    initializePlayerBitBoard(&otherPlayerBitBoard);
    for (int board = 0; board < 9; board++) {
        for (int testCaseIndex = 0; testCaseIndex < 8; testCaseIndex++) {
            for (int i = 0; i < 3; i++) {
                Square square = createSquare(board, directWins[testCaseIndex][i]);
                myAssert(!boardIsWon(&playerBitBoard, board));
                setSquareOccupied(&playerBitBoard, &otherPlayerBitBoard, square);
            }
            myAssert(boardIsWon(&playerBitBoard, board));
            initializePlayerBitBoard(&playerBitBoard);
            for (int i = 0; i < 4; i++) {
                Square square = createSquare(board, oneAdditionalMove[testCaseIndex][i]);
                myAssert(!boardIsWon(&playerBitBoard, board));
                setSquareOccupied(&playerBitBoard, &otherPlayerBitBoard, square);
            }
            myAssert(boardIsWon(&playerBitBoard, board));
            initializePlayerBitBoard(&playerBitBoard);
            for (int i = 0; i < 5; i++) {
                Square square = createSquare(board, twoAdditionalMoves[testCaseIndex][i]);
                myAssert(!boardIsWon(&playerBitBoard, board));
                setSquareOccupied(&playerBitBoard, &otherPlayerBitBoard, square);
            }
            myAssert(boardIsWon(&playerBitBoard, board));
            initializePlayerBitBoard(&playerBitBoard);
        }
    }
}


void drawSmallBoardUpdatesBothBigBoards() {
    int movesLeadingToDraw[9] = {
            0, 4,
            6, 3,
            5, 1,
            7, 8,
            2
    };
    PlayerBitBoard playerBitBoard;
    initializePlayerBitBoard(&playerBitBoard);
    PlayerBitBoard otherPlayerBitBoard;
    initializePlayerBitBoard(&otherPlayerBitBoard);
    for (uint8_t board = 0; board < 9; board++) {
        for (int i = 0; i < 9; i++) {
            Square square = createSquare(board, movesLeadingToDraw[i]);
            if (i % 2 == 0) {
                setSquareOccupied(&playerBitBoard, &otherPlayerBitBoard, square);
            } else {
                setSquareOccupied(&otherPlayerBitBoard, &playerBitBoard, square);
            }
        }
        myAssert(boardIsWon(&playerBitBoard, board) && boardIsWon(&otherPlayerBitBoard, board));
        initializePlayerBitBoard(&playerBitBoard);
    }
}


void runPlayerBitBoardTests() {
    printf("\tboardInitiallyEmpty...\n");
    boardInitiallyEmpty();
    printf("\ttestSetSquareOccupied...\n");
    testSetSquareOccupied();
    printf("\twinSmallBoardUpdatesBigBoard...\n");
    winSmallBoardUpdatesBigBoard();
    printf("\tdrawSmallBoardUpdatesBothBigBoards...\n");
    drawSmallBoardUpdatesBothBigBoards();
}
