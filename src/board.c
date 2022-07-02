#include "board.h"
#include "util.h"
#include "player_bitboard.h"


#define ANY_BOARD 9


typedef struct AdditionalState {
    Player currentPlayer;
    uint8_t currentBoard;
    Winner winner;
} AdditionalState;


typedef struct Board {
    PlayerBitBoard* player1;
    PlayerBitBoard* player2;
    AdditionalState additionalState;
    AdditionalState additionalStateCheckpoint;
} Board;


Board* createBoard() {
    Board* board = safe_malloc(sizeof(Board));
    board->player1 = createPlayerBitBoard();
    board->player2 = createPlayerBitBoard();
    board->additionalState.currentPlayer = PLAYER1;
    board->additionalState.currentBoard = ANY_BOARD;
    board->additionalState.winner = NONE;
    board->additionalStateCheckpoint = board->additionalState;
    return board;
}


void freeBoard(Board* board) {
    freePlayerBitBoard(board->player1);
    freePlayerBitBoard(board->player2);
    safe_free(board);
}


int generateMovesSingleBoard(Board* board, uint8_t boardIndex, Square moves[TOTAL_SMALL_SQUARES], int amountOfMoves) {
    uint16_t smallBoardPlayer1 = getSmallBoard(board->player1, boardIndex);
    uint16_t smallBoardPlayer2 = getSmallBoard(board->player2, boardIndex);
    uint16_t openSquares = ~(smallBoardPlayer1 | smallBoardPlayer2) & 511;
    while (openSquares) {
        Square square = {boardIndex, __builtin_ffs(openSquares) - 1};
        moves[amountOfMoves++] = square;
        openSquares &= openSquares - 1;
    }
    return amountOfMoves;
}


int generateMovesAnyBoard(Board* board, Square moves[TOTAL_SMALL_SQUARES]) {
    int amountOfMoves = 0;
    uint16_t undecidedSmallBoards = ~(getBigBoard(board->player1) | getBigBoard(board->player2)) & 511;
    while (undecidedSmallBoards) {
        uint8_t boardIndex = __builtin_ffs(undecidedSmallBoards) - 1;
        amountOfMoves = generateMovesSingleBoard(board, boardIndex, moves, amountOfMoves);
        undecidedSmallBoards &= undecidedSmallBoards - 1;
    }
    return amountOfMoves;
}


int generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES]) {
    if (board->additionalState.winner != NONE) {
        return 0;
    }
    uint8_t currentBoard = board->additionalState.currentBoard;
    return currentBoard == ANY_BOARD
        ? generateMovesAnyBoard(board, moves)
        : generateMovesSingleBoard(board, currentBoard, moves, 0);
}


void revertToCheckpoint(Board* board) {
    revertToPlayerCheckpoint(board->player1);
    revertToPlayerCheckpoint(board->player2);
    board->additionalState = board->additionalStateCheckpoint;
}


void updateCheckpoint(Board* board) {
    updatePlayerCheckpoint(board->player1);
    updatePlayerCheckpoint(board->player2);
    board->additionalStateCheckpoint = board->additionalState;
}


Winner getSmallBoardWinner(Board* board, uint8_t boardIndex) {
    bool player1Bit = boardIsWon(board->player1, boardIndex);
    bool player2Bit = boardIsWon(board->player2, boardIndex);
    return 2*player2Bit + player1Bit;
}


Occupation getSquare(Board* board, Square square) {
    bool player1Bit = squareIsOccupied(board->player1, square);
    bool player2Bit = squareIsOccupied(board->player2, square);
    return 2*player2Bit + player1Bit;
}


uint8_t getNextBoard(Board* board, uint8_t previousPosition) {
    Winner smallBoardWinner = getSmallBoardWinner(board, previousPosition);
    return smallBoardWinner == NONE ? previousPosition : ANY_BOARD;
}


void verifyWinner(Board* board) {
    uint16_t player1BigBoard = getBigBoard(board->player1);
    uint16_t player2BigBoard = getBigBoard(board->player2);
    uint16_t decisiveBoards = player1BigBoard ^ player2BigBoard;
    uint16_t boardsWonByPlayer1 = player1BigBoard & decisiveBoards;
    if (isWin(boardsWonByPlayer1)) {
        board->additionalState.winner = WIN_P1;
        return;
    }
    uint16_t boardsWonByPlayer2 = player2BigBoard & decisiveBoards;
    if (isWin(boardsWonByPlayer2)) {
        board->additionalState.winner = WIN_P2;
        return;
    }
    if ((player1BigBoard | player2BigBoard) == 511) {
        int player1AmountBoardsWon = __builtin_popcount(player1BigBoard);
        int player2AmountBoardsWon = __builtin_popcount(player2BigBoard);
        board->additionalState.winner = (
                player1AmountBoardsWon > player2AmountBoardsWon
                    ? WIN_P1
                    : player1AmountBoardsWon < player2AmountBoardsWon
                        ? WIN_P2
                        : DRAW
        );
    }
}


void makeTemporaryMove(Board* board, Square square) {
    assertMsg(
            square.board == board->additionalState.currentBoard
            || board->additionalState.currentBoard == ANY_BOARD,
            "Can't make a move on that board");
    assertMsg(getSquare(board, square) == UNOCCUPIED, "Can't make a move on a square that is already occupied");
    assertMsg(board->additionalState.winner == NONE, "Can't make a move when there is already a winner");

    bool bigBoardWasUpdated = board->additionalState.currentPlayer == PLAYER1
            ? setSquareOccupied(board->player1, board->player2, square)
            : setSquareOccupied(board->player2, board->player1, square);
    if (bigBoardWasUpdated) {
        verifyWinner(board);
    }
    board->additionalState.currentPlayer = otherPlayer(board->additionalState.currentPlayer);
    board->additionalState.currentBoard = getNextBoard(board, square.position);
}


void makePermanentMove(Board* board, Square square) {
    makeTemporaryMove(board, square);
    updateCheckpoint(board);
}


Winner getWinner(Board* board) {
    return board->additionalState.winner;
}
