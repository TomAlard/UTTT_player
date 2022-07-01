#include "bitboard.h"
#include "util.h"
#include "player_bitboard.h"


#define ANY_BOARD 9


typedef struct AdditionalState {
    Player currentPlayer;
    uint8_t currentBoard;
    Winner winner;
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
    bitBoard->additionalState.winner = NONE;
    bitBoard->additionalStateCheckpoint = bitBoard->additionalState;
    return bitBoard;
}


void freeBitBoard(BitBoard* bitBoard) {
    freePlayerBitBoard(bitBoard->player1);
    freePlayerBitBoard(bitBoard->player2);
    safe_free(bitBoard);
}


int generateMovesSingleBoard(BitBoard* bitBoard, uint8_t board, Square moves[TOTAL_SMALL_SQUARES], int amountOfMoves) {
    uint16_t smallBoardPlayer1 = getSmallBoard(bitBoard->player1, board);
    uint16_t smallBoardPlayer2 = getSmallBoard(bitBoard->player2, board);
    uint16_t openSquares = ~(smallBoardPlayer1 | smallBoardPlayer2) & 511;
    while (openSquares) {
        Square square = {board, __builtin_ffs(openSquares) - 1};
        moves[amountOfMoves++] = square;
        openSquares &= openSquares - 1;
    }
    return amountOfMoves;
}


int generateMovesAnyBoard(BitBoard* bitBoard, Square moves[TOTAL_SMALL_SQUARES]) {
    int amountOfMoves = 0;
    uint16_t undecidedSmallBoards = ~(getBigBoard(bitBoard->player1) | getBigBoard(bitBoard->player2)) & 511;
    while (undecidedSmallBoards) {
        uint8_t board = __builtin_ffs(undecidedSmallBoards) - 1;
        amountOfMoves = generateMovesSingleBoard(bitBoard, board, moves, amountOfMoves);
        undecidedSmallBoards &= undecidedSmallBoards - 1;
    }
    return amountOfMoves;
}


int generateMoves(BitBoard* bitBoard, Square moves[TOTAL_SMALL_SQUARES]) {
    if (bitBoard->additionalState.winner != NONE) {
        return 0;
    }
    uint8_t currentBoard = bitBoard->additionalState.currentBoard;
    return currentBoard == ANY_BOARD
        ? generateMovesAnyBoard(bitBoard, moves)
        : generateMovesSingleBoard(bitBoard, currentBoard, moves, 0);
}


Winner getSmallBoardWinner(BitBoard* bitBoard, uint8_t board) {
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
    Winner smallBoardWinner = getSmallBoardWinner(bitBoard, previousPosition);
    return smallBoardWinner == NONE ? previousPosition : ANY_BOARD;
}


void verifyWinner(BitBoard* bitBoard) {
    uint16_t player1BigBoard = getBigBoard(bitBoard->player1);
    uint16_t player2BigBoard = getBigBoard(bitBoard->player2);
    uint16_t decisiveBoards = player1BigBoard ^ player2BigBoard;
    uint16_t boardsWonByPlayer1 = player1BigBoard & decisiveBoards;
    if (isWin(boardsWonByPlayer1)) {
        bitBoard->additionalState.winner = WIN_P1;
        return;
    }
    uint16_t boardsWonByPlayer2 = player2BigBoard & decisiveBoards;
    if (isWin(boardsWonByPlayer2)) {
        bitBoard->additionalState.winner = WIN_P2;
        return;
    }
    if ((player1BigBoard | player2BigBoard) == 511) {
        int player1AmountBoardsWon = __builtin_popcount(player1BigBoard);
        int player2AmountBoardsWon = __builtin_popcount(player2BigBoard);
        bitBoard->additionalState.winner = (
                player1AmountBoardsWon > player2AmountBoardsWon
                    ? WIN_P1
                    : player1AmountBoardsWon < player2AmountBoardsWon
                        ? WIN_P2
                        : DRAW
        );
    }
}


Winner makeMove(BitBoard* bitBoard, Square square) {
    assertMsg(
            square.board == bitBoard->additionalState.currentBoard
            || bitBoard->additionalState.currentBoard == ANY_BOARD,
            "Can't make a move on that board");
    assertMsg(getSquare(bitBoard, square) == UNOCCUPIED, "Can't make a move on a square that is already occupied");
    assertMsg(bitBoard->additionalState.winner == NONE, "Can't make a move when there is already a winner");

    bool bigBoardWasUpdated = bitBoard->additionalState.currentPlayer == PLAYER1
            ? setSquareOccupied(bitBoard->player1, bitBoard->player2, square)
            : setSquareOccupied(bitBoard->player2, bitBoard->player1, square);
    if (bigBoardWasUpdated) {
        verifyWinner(bitBoard);
    }
    bitBoard->additionalState.currentPlayer = otherPlayer(bitBoard->additionalState.currentPlayer);
    bitBoard->additionalState.currentBoard = getNextBoard(bitBoard, square.position);
    return bitBoard->additionalState.winner;
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
