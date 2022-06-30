#include "player_bitboard.h"
#include "util.h"


typedef struct PlayerBitBoard {
    uint16_t smallBoards[9];
    uint16_t bigBoard;
    uint16_t checkpointSmallBoards[9];
    uint16_t checkpointBigBoard;
} PlayerBitBoard;


PlayerBitBoard* createPlayerBitBoard() {
    return safe_calloc(sizeof(PlayerBitBoard));
}


void freePlayerBitBoard(PlayerBitBoard* playerBitBoard) {
    safe_free(playerBitBoard);
}


uint16_t getBigBoard(PlayerBitBoard* playerBitBoard) {
    return playerBitBoard->bigBoard;
}


bool boardIsWon(PlayerBitBoard* playerBitBoard, uint8_t board) {
    return BIT_CHECK(playerBitBoard->bigBoard, board);
}


uint16_t getSmallBoard(PlayerBitBoard* playerBitBoard, uint8_t board) {
    return playerBitBoard->smallBoards[board];
}


bool squareIsOccupied(PlayerBitBoard* playerBitBoard, Square square) {
    return BIT_CHECK(playerBitBoard->smallBoards[square.board], square.position);
}


uint16_t winningMasks[8] = {448, 56, 7, 292, 146, 73, 273, 84};  // in order: 3 horizontal, 3 vertical, 2 diagonal
bool isWin(uint16_t smallBoard) {
    for (int i = 0; i < 8; i++) {
        uint16_t winningMask = winningMasks[i];
        if ((smallBoard & winningMask) == winningMask) {
            return true;
        }
    }
    return false;
}


bool isDraw(uint16_t smallBoard, uint16_t otherPlayerSmallBoard) {
    return (smallBoard | otherPlayerSmallBoard) == 511;
}


void setSquareOccupied(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayerBitBoard, Square square) {
    BIT_SET(playerBitBoard->smallBoards[square.board], square.position);
    uint16_t smallBoard = playerBitBoard->smallBoards[square.board];
    uint16_t otherPlayerSmallBoard = otherPlayerBitBoard->smallBoards[square.board];
    if (isWin(smallBoard)) {
        BIT_SET(playerBitBoard->bigBoard, square.board);
    } else if (isDraw(smallBoard, otherPlayerSmallBoard)) {
        BIT_SET(playerBitBoard->bigBoard, square.board);
        BIT_SET(otherPlayerBitBoard->bigBoard, square.board);
    }
}


void revertToPlayerCheckpoint(PlayerBitBoard* playerBitBoard) {
    for (int i = 0; i < 9; i++) {
        playerBitBoard->smallBoards[i] = playerBitBoard->checkpointSmallBoards[i];
    }
    playerBitBoard->bigBoard = playerBitBoard->checkpointBigBoard;
}


void updatePlayerCheckpoint(PlayerBitBoard* playerBitBoard) {
    for (int i = 0; i < 9; i++) {
        playerBitBoard->checkpointSmallBoards[i] = playerBitBoard->smallBoards[i];
    }
    playerBitBoard->checkpointBigBoard = playerBitBoard->bigBoard;
}
