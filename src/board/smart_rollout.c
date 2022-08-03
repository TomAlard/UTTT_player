#include <string.h>
#include "smart_rollout.h"
#include "../misc/util.h"


uint16_t precalculatedInstantWinBoards[512];
uint16_t calculateInstantWinBoards_(uint16_t bigBoard) {
    uint16_t lineAlreadyFilled[8] = {0x7, 0x38, 0x1c0, 0x49, 0x92, 0x124, 0x111, 0x54};
    uint16_t hasInstantWinBoardMasks[24] = {
            3, 5, 6,
            24, 40, 48,
            192, 320, 384,
            9, 65, 72,
            18, 130, 144,
            36, 260, 288,
            17, 257, 272,
            20, 68, 80
    };
    uint16_t instantWinBoard[24] = {
            2, 1, 0,
            5, 4, 3,
            8, 7, 6,
            6, 3, 0,
            7, 4, 1,
            8, 5, 2,
            8, 4, 0,
            6, 4, 2
    };
    uint16_t result = 0;
    for (int i = 0; i < 24; i++) {
        uint16_t m1 = hasInstantWinBoardMasks[i];
        uint16_t m2 = lineAlreadyFilled[i/3];
        if ((bigBoard & m1) == m1 && (bigBoard & m2) != m2) {
            BIT_SET(result, instantWinBoard[i]);
        }
    }
    return result;
}


uint16_t calculateInstantWinBoards(uint16_t bigBoard, uint16_t otherPlayerBigBoard) {
    return precalculatedInstantWinBoards[bigBoard & ~otherPlayerBigBoard] & ~otherPlayerBigBoard;
}


bool smallBoardHasInstantWinMove(uint16_t smallBoard, uint16_t otherSmallBoard) {
    return calculateInstantWinBoards(smallBoard, otherSmallBoard) != 0;
}


void initializeLookupTable() {
    for (uint16_t bigBoard = 0; bigBoard < 512; bigBoard++) {
        precalculatedInstantWinBoards[bigBoard] = calculateInstantWinBoards_(bigBoard);
    }
}


void initializeRolloutState(RolloutState* RS) {
    memset(RS, 0, sizeof(RolloutState));
}


bool canWinWith3InARow(uint8_t currentBoard, uint16_t smallBoardsWithWinningMove) {
    return (currentBoard == ANY_BOARD && smallBoardsWithWinningMove)
        || BIT_CHECK(smallBoardsWithWinningMove, currentBoard);
}


// https://stackoverflow.com/a/28303898
bool numberIsPowerOf2(uint16_t n) {
    return (n & (n - 1)) == 0;
}


bool onlyOneSmallBoardOpen(uint16_t openSmallBoards) {
    return numberIsPowerOf2(openSmallBoards);
}


bool canWinLastBoard(uint16_t lastBoard, uint8_t currentBoard, uint16_t instantWinSmallBoards) {
    return (instantWinSmallBoards & lastBoard)
        && (currentBoard == ANY_BOARD || BIT_CHECK(lastBoard, currentBoard));
}


bool hasMoreSmallBoardsThanOpponentWin(Board* board, Player player) {
    int player1SmallBoards = __builtin_popcount(board->player1.bigBoard) + !player;
    int player2SmallBoards = __builtin_popcount(board->player2.bigBoard) + player;
    return (player == PLAYER1 && player1SmallBoards > player2SmallBoards)
        || (player == PLAYER2 && player1SmallBoards < player2SmallBoards);
}


bool winsGameByWinningLastBoard(Board* board, uint16_t lastBoard, uint8_t currentBoard, uint16_t instantWinSmallBoards, Player player) {
    return canWinLastBoard(lastBoard, currentBoard, instantWinSmallBoards)
        && hasMoreSmallBoardsThanOpponentWin(board, player);
}


bool canWinWithMoreSmallBoards(Board* board, uint8_t currentBoard, uint16_t instantWinSmallBoards, Player player) {
    uint16_t openSmallBoards = 511 - (board->player1.bigBoard | board->player2.bigBoard);
    return onlyOneSmallBoardOpen(openSmallBoards)
           && winsGameByWinningLastBoard(board, openSmallBoards, currentBoard, instantWinSmallBoards, player);
}


bool hasWinningMove(Board* board, RolloutState* RS) {
    Player player = getCurrentPlayer(board);
    uint8_t currentBoard = getCurrentBoard(board);
    uint16_t smallBoardsWithWinningMove = RS->instantWinBoards[player] & RS->instantWinSmallBoards[player];
    return canWinWith3InARow(currentBoard, smallBoardsWithWinningMove)
        || canWinWithMoreSmallBoards(board, currentBoard, RS->instantWinSmallBoards[player], player);
}


void updateSmallBoardState(RolloutState* RS, uint8_t boardIndex, uint16_t player1SmallBoard, uint16_t player2SmallBoard) {
    BIT_CHANGE(RS->instantWinSmallBoards[PLAYER1], boardIndex, smallBoardHasInstantWinMove(player1SmallBoard, player2SmallBoard));
    BIT_CHANGE(RS->instantWinSmallBoards[PLAYER2], boardIndex, smallBoardHasInstantWinMove(player2SmallBoard, player1SmallBoard));
}


void updateBigBoardState(RolloutState* RS, uint16_t player1BigBoard, uint16_t player2BigBoard) {
    RS->instantWinBoards[PLAYER1] = calculateInstantWinBoards(player1BigBoard, player2BigBoard);
    RS->instantWinBoards[PLAYER2] = calculateInstantWinBoards(player2BigBoard, player1BigBoard);
}
