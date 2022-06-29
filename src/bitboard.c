#include "bitboard.h"
#include "util.h"
#include "player_bitboard.h"


typedef struct BitBoard {
    PlayerBitBoard* player1;
    PlayerBitBoard* player2;
    Player currentPlayer;
} BitBoard;


BitBoard* createBitBoard() {
    BitBoard* bitBoard = safe_malloc(sizeof(BitBoard));
    bitBoard->player1 = createPlayerBitBoard();
    bitBoard->player2 = createPlayerBitBoard();
    bitBoard->currentPlayer = PLAYER1;
    return bitBoard;
}


void freeBitBoard(BitBoard* bitBoard) {
    freePlayerBitBoard(bitBoard->player1);
    freePlayerBitBoard(bitBoard->player2);
    safe_free(bitBoard);
}


Player getBigBoardSquare(BitBoard* bitBoard, uint8_t board) {
    bool player1Bit = boardIsWon(bitBoard->player1, board);
    bool player2Bit = boardIsWon(bitBoard->player2, board);
    return 2*player2Bit + player1Bit;
}


Player getSquare(BitBoard* bitBoard, Square square) {
    bool player1Bit = squareIsOccupied(bitBoard->player1, square);
    bool player2Bit = squareIsOccupied(bitBoard->player2, square);
    return 2*player2Bit + player1Bit;
}


void makeMove(BitBoard* bitBoard, Square square) {
    if (bitBoard->currentPlayer == PLAYER1) {
        setSquareOccupied(bitBoard->player1, bitBoard->player2, square);
    } else {
        setSquareOccupied(bitBoard->player2, bitBoard->player1, square);
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
