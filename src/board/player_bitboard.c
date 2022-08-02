#pragma GCC target("avx")


#include <emmintrin.h>
#include <smmintrin.h>
#include <string.h>
#include "player_bitboard.h"
#include "../misc/util.h"


bool precalculatedIsWin[512];
bool isWin_(uint16_t smallBoard) {
    __m128i masks = _mm_setr_epi16(0x7, 0x38, 0x1c0, 0x49, 0x92, 0x124, 0x111, 0x54);
    __m128i v1 = _mm_set1_epi16(1);
    __m128i boards128 = _mm_set1_epi16((short) smallBoard);
    __m128i andResult = _mm_and_si128(masks, boards128);
    __m128i result = _mm_cmpeq_epi16(andResult, masks);
    return !_mm_test_all_zeros(result, v1);
}


bool isWin(uint16_t smallBoard) {
    return precalculatedIsWin[smallBoard];
}


PlayerBitBoard* createPlayerBitBoard() {
    for (uint16_t i = 0; i < 512; i++) {
        precalculatedIsWin[i] = isWin_(i);
    }
    return safe_calloc(sizeof(PlayerBitBoard));
}


void initializePlayerBitBoard(PlayerBitBoard* playerBitBoard) {
    for (uint16_t i = 0; i < 512; i++) {
        precalculatedIsWin[i] = isWin_(i);
    }
    memset(playerBitBoard, 0, sizeof(PlayerBitBoard));
}


void freePlayerBitBoard(PlayerBitBoard* playerBitBoard) {
    safe_free(playerBitBoard);
}


bool boardIsWon(PlayerBitBoard* playerBitBoard, uint8_t board) {
    return BIT_CHECK(playerBitBoard->bigBoard, board);
}


bool squareIsOccupied(PlayerBitBoard* playerBitBoard, Square square) {
    return BIT_CHECK(playerBitBoard->smallBoards[square.board], square.position);
}


bool isDraw(uint16_t smallBoard, uint16_t otherPlayerSmallBoard) {
    return (smallBoard | otherPlayerSmallBoard) == 511;
}


bool setSquareOccupied(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayerBitBoard, Square square) {
    BIT_SET(playerBitBoard->smallBoards[square.board], square.position);
    uint16_t smallBoard = playerBitBoard->smallBoards[square.board];
    uint16_t otherPlayerSmallBoard = otherPlayerBitBoard->smallBoards[square.board];
    if (isWin(smallBoard)) {
        BIT_SET(playerBitBoard->bigBoard, square.board);
        return true;
    } if (isDraw(smallBoard, otherPlayerSmallBoard)) {
        BIT_SET(playerBitBoard->bigBoard, square.board);
        BIT_SET(otherPlayerBitBoard->bigBoard, square.board);
        return true;
    }
    return false;
}


void revertToPlayerCheckpoint(PlayerBitBoard* playerBitBoard) {
    memcpy(playerBitBoard->smallBoards, playerBitBoard->checkpointSmallBoards, 9 * sizeof(uint16_t));
    playerBitBoard->bigBoard = playerBitBoard->checkpointBigBoard;
}


void updatePlayerCheckpoint(PlayerBitBoard* playerBitBoard) {
    memcpy(playerBitBoard->checkpointSmallBoards, playerBitBoard->smallBoards, 9 * sizeof(uint16_t));
    playerBitBoard->checkpointBigBoard = playerBitBoard->bigBoard;
}
