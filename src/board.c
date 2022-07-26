#include <string.h>
#include <assert.h>
#include "board.h"
#include "util.h"
#include "player_bitboard.h"


#define ANY_BOARD 9


typedef struct AdditionalState {
    Player currentPlayer;
    uint8_t currentBoard;
    Winner winner;
    uint8_t ply;
    uint8_t totalAmountOfOpenSquares;
    uint8_t amountOfOpenSquaresBySmallBoard[9];
} AdditionalState;


typedef struct Board {
    PlayerBitBoard* player1;
    PlayerBitBoard* player2;
    AdditionalState AS;
    AdditionalState ASCheckpoint;
    Square openSquares[512][9][9];
    int8_t amountOfOpenSquares[512];
    Player me;
} Board;


int8_t setOpenSquares(Square openSquares[9], uint8_t boardIndex, uint16_t bitBoard) {
    int8_t amountOfMoves = 0;
    while (bitBoard) {
        Square square = {boardIndex, __builtin_ffs(bitBoard) - 1};
        openSquares[amountOfMoves++] = square;
        bitBoard &= bitBoard - 1;
    }
    return amountOfMoves;
}


Board* createBoard() {
    Board* board = safe_malloc(sizeof(Board));
    board->player1 = createPlayerBitBoard();
    board->player2 = createPlayerBitBoard();
    board->AS.currentPlayer = PLAYER1;
    board->AS.currentBoard = ANY_BOARD;
    board->AS.winner = NONE;
    board->AS.ply = 1;
    board->AS.totalAmountOfOpenSquares = 81;
    for (int boardIndex = 0; boardIndex < 9; boardIndex++) {
        board->AS.amountOfOpenSquaresBySmallBoard[boardIndex] = 9;
    }
    board->ASCheckpoint = board->AS;
    for (int boardIndex = 0; boardIndex < 9; boardIndex++) {
        for (int bitBoard = 0; bitBoard < 512; bitBoard++) {
            board->amountOfOpenSquares[bitBoard] =
                    setOpenSquares(board->openSquares[bitBoard][boardIndex], boardIndex, bitBoard);
        }
    }
    board->me = PLAYER2;
    return board;
}


void freeBoard(Board* board) {
    freePlayerBitBoard(board->player1);
    freePlayerBitBoard(board->player2);
    safe_free(board);
}


Square* getMovesSingleBoard(Board* board, uint8_t boardIndex, int8_t* amountOfMoves) {
    uint16_t bitBoard = ~(getSmallBoard(board->player1, boardIndex) | getSmallBoard(board->player2, boardIndex)) & 511;
    *amountOfMoves = board->amountOfOpenSquares[bitBoard];
    return board->openSquares[bitBoard][boardIndex];
}


int8_t copyMovesSingleBoard(Board* board, uint8_t boardIndex, Square moves[TOTAL_SMALL_SQUARES], int8_t amountOfMoves) {
    uint16_t bitBoard = ~(getSmallBoard(board->player1, boardIndex) | getSmallBoard(board->player2, boardIndex)) & 511;
    memcpy(&moves[amountOfMoves], board->openSquares[bitBoard][boardIndex],
           board->amountOfOpenSquares[bitBoard] * sizeof(Square));
    return (int8_t)(amountOfMoves + board->amountOfOpenSquares[bitBoard]);
}


int8_t generateMovesAnyBoard(Board* board, Square moves[TOTAL_SMALL_SQUARES]) {
    int8_t amountOfMoves = 0;
    uint16_t undecidedSmallBoards = ~(getBigBoard(board->player1) | getBigBoard(board->player2)) & 511;
    while (undecidedSmallBoards) {
        uint8_t boardIndex = __builtin_ffs(undecidedSmallBoards) - 1;
        amountOfMoves = copyMovesSingleBoard(board, boardIndex, moves, amountOfMoves);
        undecidedSmallBoards &= undecidedSmallBoards - 1;
    }
    return amountOfMoves;
}


Square* generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves) {
    if (board->AS.winner != NONE) {
        *amountOfMoves = 0;
        return moves;
    }
    uint8_t currentBoard = board->AS.currentBoard;
    if (currentBoard == ANY_BOARD) {
        *amountOfMoves = generateMovesAnyBoard(board, moves);
        return moves;
    }
    return getMovesSingleBoard(board, currentBoard, amountOfMoves);
}


void makeRandomTemporaryMove(Board* board, RNG* rng) {
    assert(board->AS.winner == NONE && "makeRandomTemporaryMove: there is already a winner");
    uint8_t currentBoard = board->AS.currentBoard;
    uint8_t randomMoveIndex;
    if (currentBoard == ANY_BOARD) {
        uint8_t randomMultipleBoardMoveIndex = generateBoundedRandomNumber(rng, board->AS.totalAmountOfOpenSquares);
        int boardIndex = 0;
        int cumulativeSum = 0;
        while (boardIndex < 9 && cumulativeSum <= randomMultipleBoardMoveIndex) {
            cumulativeSum += board->AS.amountOfOpenSquaresBySmallBoard[boardIndex++];
        }
        cumulativeSum -= board->AS.amountOfOpenSquaresBySmallBoard[--boardIndex];
        randomMoveIndex = randomMultipleBoardMoveIndex - cumulativeSum;
        currentBoard = boardIndex;
    } else {
        randomMoveIndex = generateBoundedRandomNumber(rng, board->AS.amountOfOpenSquaresBySmallBoard[currentBoard]);
    }
    int8_t amountOfMoves;
    Square* moves = getMovesSingleBoard(board, currentBoard, &amountOfMoves);
    Square randomMove = moves[randomMoveIndex];
    makeTemporaryMove(board, randomMove);
}


bool nextBoardIsEmpty(Board* board) {
    uint8_t currentBoard = board->AS.currentBoard;
    return currentBoard != ANY_BOARD
        && (getSmallBoard(board->player1, currentBoard) | getSmallBoard(board->player2, currentBoard)) == 0;
}


uint8_t getCurrentBoard(Board* board) {
    return board->AS.currentBoard;
}


void revertToCheckpoint(Board* board) {
    revertToPlayerCheckpoint(board->player1);
    revertToPlayerCheckpoint(board->player2);
    board->AS = board->ASCheckpoint;
}


void updateCheckpoint(Board* board) {
    updatePlayerCheckpoint(board->player1);
    updatePlayerCheckpoint(board->player2);
    board->ASCheckpoint = board->AS;
}


Occupation getSquare(Board* board, Square square) {
    bool player1Bit = squareIsOccupied(board->player1, square);
    bool player2Bit = squareIsOccupied(board->player2, square);
    return 2*player2Bit + player1Bit;
}


uint8_t getNextBoard(Board* board, uint8_t previousPosition) {
    bool smallBoardIsDecided = boardIsWon(board->player1, previousPosition) || boardIsWon(board->player2, previousPosition);
    return smallBoardIsDecided ? ANY_BOARD : previousPosition;
}


Winner calculateWinner(Board* board) {
    uint16_t player1BigBoard = getBigBoard(board->player1);
    uint16_t player2BigBoard = getBigBoard(board->player2);
    uint16_t decisiveBoards = player1BigBoard ^ player2BigBoard;
    uint16_t boardsWonByPlayer1 = player1BigBoard & decisiveBoards;
    if (isWin(boardsWonByPlayer1)) {
        return WIN_P1;
    }
    uint16_t boardsWonByPlayer2 = player2BigBoard & decisiveBoards;
    if (isWin(boardsWonByPlayer2)) {
        return WIN_P2;
    }
    if ((player1BigBoard | player2BigBoard) == 511) {
        int player1AmountBoardsWon = __builtin_popcount(player1BigBoard);
        int player2AmountBoardsWon = __builtin_popcount(player2BigBoard);
        return player1AmountBoardsWon > player2AmountBoardsWon
                    ? WIN_P1
                    : player1AmountBoardsWon < player2AmountBoardsWon
                        ? WIN_P2
                        : DRAW;
    }
    return NONE;
}


void makeTemporaryMove(Board* board, Square square) {
    assert(
            square.board == board->AS.currentBoard
            || board->AS.currentBoard == ANY_BOARD &&
            "Can't make a move on that board");
    assert(getSquare(board, square) == UNOCCUPIED && "Can't make a move on a square that is already occupied");
    assert(board->AS.winner == NONE && "Can't make a move when there is already a winner");

    bool bigBoardWasUpdated = board->AS.currentPlayer == PLAYER1
            ? setSquareOccupied(board->player1, board->player2, square)
            : setSquareOccupied(board->player2, board->player1, square);
    if (bigBoardWasUpdated) {
        board->AS.winner = calculateWinner(board);
        board->AS.totalAmountOfOpenSquares -= board->AS.amountOfOpenSquaresBySmallBoard[square.board];
        board->AS.amountOfOpenSquaresBySmallBoard[square.board] = 0;
    } else {
        board->AS.totalAmountOfOpenSquares--;
        board->AS.amountOfOpenSquaresBySmallBoard[square.board]--;
    }
    board->AS.currentPlayer = otherPlayer(board->AS.currentPlayer);
    board->AS.currentBoard = getNextBoard(board, square.position);
    board->AS.ply++;
}


void makePermanentMove(Board* board, Square square) {
    makeTemporaryMove(board, square);
    updateCheckpoint(board);
}


Winner getWinner(Board* board) {
    return board->AS.winner;
}


void setMe(Board* board, Player player) {
    board->me = player;
    board->AS.ply = player == PLAYER2;
    board->ASCheckpoint.ply = player == PLAYER2;
}


bool currentPlayerIsMe(Board* board) {
    return board->AS.currentPlayer == board->me;
}


uint8_t getPly(Board* board) {
    return board->AS.ply;
}
