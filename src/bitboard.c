#include "bitboard.h"
#include "util.h"
#include "player_bitboard.h"


#define ANY_BOARD 9


typedef struct AdditionalState {
    Player currentPlayer;
    uint8_t currentBoard;
} AdditionalState;


typedef struct BitBoard {
    PlayerBitBoard* player1;
    PlayerBitBoard* player2;
    AdditionalState additionalState;
    AdditionalState additionalStateCheckpoint;
} BitBoard;


BitBoard* createBitBoard() {
    BitBoard* bitBoard = safe_malloc(sizeof(BitBoard));
    bitBoard->player1 = createPlayerBitBoard();
    bitBoard->player2 = createPlayerBitBoard();
    bitBoard->additionalState.currentPlayer = PLAYER1;
    bitBoard->additionalState.currentBoard = ANY_BOARD;
    bitBoard->additionalStateCheckpoint = bitBoard->additionalState;
    return bitBoard;
}


void freeBitBoard(BitBoard* bitBoard) {
    freePlayerBitBoard(bitBoard->player1);
    freePlayerBitBoard(bitBoard->player2);
    safe_free(bitBoard);
}


SmallBoardState getBigBoardSquare(BitBoard* bitBoard, uint8_t board) {
    bool player1Bit = boardIsWon(bitBoard->player1, board);
    bool player2Bit = boardIsWon(bitBoard->player2, board);
    return 2*player2Bit + player1Bit;
}


Occupation getSquare(BitBoard* bitBoard, Square square) {
    bool player1Bit = squareIsOccupied(bitBoard->player1, square);
    bool player2Bit = squareIsOccupied(bitBoard->player2, square);
    return 2*player2Bit + player1Bit;
}


uint8_t getNextBoard(BitBoard* bitBoard, uint8_t previousPosition) {
    SmallBoardState nextBoardState = getBigBoardSquare(bitBoard, previousPosition);
    return nextBoardState == UNDECIDED ? previousPosition : ANY_BOARD;
}


void makeMove(BitBoard* bitBoard, Square square) {
    assertMsg(
            square.board == bitBoard->additionalState.currentBoard
            || bitBoard->additionalState.currentBoard == ANY_BOARD,
            "Can't make a move on that board");
    assertMsg(getSquare(bitBoard, square) == UNOCCUPIED,
              "Can't make a move on a square that is already occupied");

    if (bitBoard->additionalState.currentPlayer == PLAYER1) {
        setSquareOccupied(bitBoard->player1, bitBoard->player2, square);
    } else {
        setSquareOccupied(bitBoard->player2, bitBoard->player1, square);
    }
    bitBoard->additionalState.currentPlayer = otherPlayer(bitBoard->additionalState.currentPlayer);
    bitBoard->additionalState.currentBoard = getNextBoard(bitBoard, square.position);
}


void revertToCheckpoint(BitBoard* bitBoard) {
    revertToPlayerCheckpoint(bitBoard->player1);
    revertToPlayerCheckpoint(bitBoard->player2);
    bitBoard->additionalState = bitBoard->additionalStateCheckpoint;
}


void updateCheckpoint(BitBoard* bitBoard) {
    updatePlayerCheckpoint(bitBoard->player1);
    updatePlayerCheckpoint(bitBoard->player2);
    bitBoard->additionalStateCheckpoint = bitBoard->additionalState;
}
