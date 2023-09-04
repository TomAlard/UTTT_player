#pragma GCC target("avx")


#include <emmintrin.h>
#include <smmintrin.h>
#include <string.h>
#include "player_bitboard.h"
#include "../misc/util.h"


bool isWin(uint16_t smallBoard) {
    __m128i masks = _mm_setr_epi16(0x7, 0x38, 0x1c0, 0x49, 0x92, 0x124, 0x111, 0x54);
    __m128i v1 = _mm_set1_epi16(1);
    __m128i boards128 = _mm_set1_epi16((short) smallBoard);
    __m128i andResult = _mm_and_si128(masks, boards128);
    __m128i result = _mm_cmpeq_epi16(andResult, masks);
    return !_mm_test_all_zeros(result, v1);
}


void initializePlayerBitBoard(PlayerBitBoard* playerBitBoard) {
    memset(playerBitBoard, 0, sizeof(PlayerBitBoard));
}


bool isDraw(uint16_t smallBoard, uint16_t otherPlayerSmallBoard) {
    return (smallBoard | otherPlayerSmallBoard) == 511;
}


bool setSquareOccupied(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayerBitBoard, Square square) {
    BIT_SET_128(playerBitBoard->marks, 9*square.board + square.position);
    uint16_t smallBoard = extractSmallBoard(playerBitBoard, square.board);
    uint16_t otherPlayerSmallBoard = extractSmallBoard(otherPlayerBitBoard, square.board);
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


uint16_t extractSmallBoard(PlayerBitBoard* playerBitBoard, uint8_t smallBoardIndex) {
    return (playerBitBoard->marks >> (9 * smallBoardIndex)) & 511;
}
