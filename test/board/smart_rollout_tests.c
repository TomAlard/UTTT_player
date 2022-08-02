#include <stdio.h>
#include "smart_rollout_tests.h"
#include "../../src/board/smart_rollout.h"
#include "../../src/board/board.h"
#include "../test_util.h"
#include "../../src/board/player_bitboard.h"


void hasWinningMoveTest() {
    RolloutState RS;
    initializeRolloutState(&RS);
    RNG rng;
    seedRNG(&rng, 69, 420);
    Board* board = createBoard();
    PlayerBitBoard player1;
    initializePlayerBitBoard(&player1);
    PlayerBitBoard player2;
    initializePlayerBitBoard(&player2);
    while (getWinner(board) == NONE) {
        int8_t amountOfMoves;
        Square movesArray[TOTAL_SMALL_SQUARES];
        Square* moves = generateMoves(board, movesArray, &amountOfMoves);
        bool winningMoveExists = false;
        Player currentPlayer = getCurrentPlayer(board);
        for (int i = 0; i < amountOfMoves; i++) {
            makeTemporaryMove(board, moves[i]);
            if (currentPlayer + 1 == getWinner(board)) {
                winningMoveExists = true;
            }
            revertToCheckpoint(board);
        }
        myAssert(hasWinningMove(&RS, getCurrentBoard(board), currentPlayer) == winningMoveExists);
        Square move = moves[generateBoundedRandomNumber(&rng, amountOfMoves)];
        PlayerBitBoard* player = currentPlayer == PLAYER1? &player1 : &player2;
        PlayerBitBoard* otherPlayer = currentPlayer == PLAYER1? &player2 : &player1;
        bool bigBoardWasUpdated = setSquareOccupied(player, otherPlayer, move);
        if (bigBoardWasUpdated) {
            updateBigBoardState(&RS, player1.bigBoard, player2.bigBoard);
        }
        updateSmallBoardState(&RS, move.board, player1.smallBoards[move.board], player2.smallBoards[move.board]);
        makePermanentMove(board, move);
    }
    freeBoard(board);
}


void runSmartRolloutTests() {
    printf("\thasWinningMoveTest...\n");
    hasWinningMoveTest();
}