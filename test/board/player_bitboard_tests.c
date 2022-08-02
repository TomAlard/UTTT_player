#include <stdio.h>
#include "player_bitboard_tests.h"
#include "../../src/board/player_bitboard.h"
#include "../test_util.h"


void boardInitiallyEmpty(PlayerBitBoard* playerBitBoard) {
    for (int board = 0; board < 9; board++) {
        for (int position = 0; position < 9; position++) {
            Square square = createSquare(board, position);
            myAssert(!squareIsOccupied(playerBitBoard, square));
        }
    }
}


void testSetSquareOccupied(PlayerBitBoard* playerBitBoard) {
    PlayerBitBoard* otherPlayerBitBoard = createPlayerBitBoard();
    for (int board = 0; board < 9; board++) {
        for (int position = 0; position < 9; position++) {
            Square square = createSquare(board, position);
            myAssert(!squareIsOccupied(playerBitBoard, square));
            setSquareOccupied(playerBitBoard, otherPlayerBitBoard, square);
            myAssert(squareIsOccupied(playerBitBoard, square));
        }
    }
    freePlayerBitBoard(otherPlayerBitBoard);
    revertToPlayerCheckpoint(playerBitBoard);
}


void winSmallBoardUpdatesBigBoard(PlayerBitBoard* playerBitBoard) {
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
    PlayerBitBoard* otherPlayerBitBoard = createPlayerBitBoard();
    for (int board = 0; board < 9; board++) {
        for (int testCaseIndex = 0; testCaseIndex < 8; testCaseIndex++) {
            for (int i = 0; i < 3; i++) {
                Square square = createSquare(board, directWins[testCaseIndex][i]);
                myAssert(!boardIsWon(playerBitBoard, board));
                setSquareOccupied(playerBitBoard, otherPlayerBitBoard, square);
            }
            myAssert(boardIsWon(playerBitBoard, board));
            revertToPlayerCheckpoint(playerBitBoard);
            for (int i = 0; i < 4; i++) {
                Square square = createSquare(board, oneAdditionalMove[testCaseIndex][i]);
                myAssert(!boardIsWon(playerBitBoard, board));
                setSquareOccupied(playerBitBoard, otherPlayerBitBoard, square);
            }
            myAssert(boardIsWon(playerBitBoard, board));
            revertToPlayerCheckpoint(playerBitBoard);
            for (int i = 0; i < 5; i++) {
                Square square = createSquare(board, twoAdditionalMoves[testCaseIndex][i]);
                myAssert(!boardIsWon(playerBitBoard, board));
                setSquareOccupied(playerBitBoard, otherPlayerBitBoard, square);
            }
            myAssert(boardIsWon(playerBitBoard, board));
            revertToPlayerCheckpoint(playerBitBoard);
        }
    }
    freePlayerBitBoard(otherPlayerBitBoard);
}


void drawSmallBoardUpdatesBothBigBoards(PlayerBitBoard* playerBitBoard) {
    int movesLeadingToDraw[9] = {
            0, 4,
            6, 3,
            5, 1,
            7, 8,
            2
    };
    PlayerBitBoard* otherPlayerBitBoard = createPlayerBitBoard();
    for (uint8_t board = 0; board < 9; board++) {
        for (int i = 0; i < 9; i++) {
            Square square = createSquare(board, movesLeadingToDraw[i]);
            if (i % 2 == 0) {
                setSquareOccupied(playerBitBoard, otherPlayerBitBoard, square);
            } else {
                setSquareOccupied(otherPlayerBitBoard, playerBitBoard, square);
            }
        }
        myAssert(boardIsWon(playerBitBoard, board) && boardIsWon(otherPlayerBitBoard, board));
        revertToPlayerCheckpoint(playerBitBoard);
    }
    freePlayerBitBoard(otherPlayerBitBoard);
}


void testCheckpoints(PlayerBitBoard* playerBitBoard) {
    PlayerBitBoard* otherPlayerBitBoard = createPlayerBitBoard();
    Square beforeCheckpoint = createSquare(5, 2);
    setSquareOccupied(playerBitBoard, otherPlayerBitBoard, beforeCheckpoint);
    updatePlayerCheckpoint(playerBitBoard);
    Square afterCheckpoint = createSquare(1, 4);
    setSquareOccupied(playerBitBoard, otherPlayerBitBoard, afterCheckpoint);
    revertToPlayerCheckpoint(playerBitBoard);
    myAssert(squareIsOccupied(playerBitBoard, beforeCheckpoint) && !squareIsOccupied(playerBitBoard, afterCheckpoint));
    freePlayerBitBoard(otherPlayerBitBoard);
}


void runPlayerBitBoardTests() {
    PlayerBitBoard* playerBitBoard = createPlayerBitBoard();
    printf("\tboardInitiallyEmpty...\n");
    boardInitiallyEmpty(playerBitBoard);
    printf("\ttestSetSquareOccupied...\n");
    testSetSquareOccupied(playerBitBoard);
    printf("\twinSmallBoardUpdatesBigBoard...\n");
    winSmallBoardUpdatesBigBoard(playerBitBoard);
    printf("\tdrawSmallBoardUpdatesBothBigBoards...\n");
    drawSmallBoardUpdatesBothBigBoards(playerBitBoard);
    printf("\ttestCheckpoints...\n");
    testCheckpoints(playerBitBoard);
    freePlayerBitBoard(playerBitBoard);
}
