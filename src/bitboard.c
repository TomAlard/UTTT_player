#include "bitboard.h"
#include "util.h"
#include "player_bitboard.h"


typedef struct BitBoard {
    PlayerBitBoard* player1;
    PlayerBitBoard* player2;
} BitBoard;


BitBoard* createBitBoard() {
    BitBoard* bitBoard = safe_malloc(sizeof(BitBoard));
    bitBoard->player1 = createPlayerBitBoard();
    bitBoard->player2 = createPlayerBitBoard();
    return bitBoard;
}


void freeBitBoard(BitBoard* bitBoard) {
    freePlayerBitBoard(bitBoard->player1);
    freePlayerBitBoard(bitBoard->player2);
    safe_free(bitBoard);
}


Player getBigBoardSquare(BitBoard* bitBoard, uint8_t board) {
    bool player1Bit = getPlayerBigBoardSquare(bitBoard->player1, board);
    bool player2Bit = getPlayerBigBoardSquare(bitBoard->player2, board);
    return 2*player2Bit + player1Bit;
}


Player getSquare(BitBoard* bitBoard, Square square) {
    bool player1Bit = getPlayerSquare(bitBoard->player1, square);
    bool player2Bit = getPlayerSquare(bitBoard->player2, square);
    return 2*player2Bit + player1Bit;
}


void setSquare(BitBoard* bitBoard, Square square, Player player) {
    if (player == PLAYER1) {
        setPlayerSquare(bitBoard->player1, bitBoard->player2, square);
    } else if (player == PLAYER2) {
        setPlayerSquare(bitBoard->player2, bitBoard->player1, square);
    } else if (player == NONE) {
        clearPlayerSquare(bitBoard->player1, square);
        clearPlayerSquare(bitBoard->player2, square);
    } else {
        crash("Can't pass BOTH Player Enum value to setSquare method.");
    }
}


void revertToCheckpoint(BitBoard* bitBoard) {
    revertToPlayerCheckpoint(bitBoard->player1);
    revertToPlayerCheckpoint(bitBoard->player2);
}


void updateCheckpoint(BitBoard* bitBoard) {
    updatePlayerCheckpoint(bitBoard->player1);
    updatePlayerCheckpoint(bitBoard->player2);
}


// DELETE THIS
void clearBoard(BitBoard* bitBoard) {
    clearPlayerBoard(bitBoard->player1);
    clearPlayerBoard(bitBoard->player2);
}
