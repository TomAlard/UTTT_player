#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "../misc/player.h"
#include "../main.h"
#include "arena_opponent.h"
#include "../misc/util.h"


#define ROUNDS 4000
#define TIME 0.05


Winner simulateSingleGame(bool weArePlayer1) {
    Board* board = createBoard();
    int rootIndex = createMCTSRootNode(board);
    StateOpponent* stateOpponent = initializeStateOpponent();
    Square previousMove = {9, 9};
    bool weAreCurrentPlayer = weArePlayer1;
    while (board->state.winner == NONE && stateOpponent != NULL) {
        if (weAreCurrentPlayer) {
            previousMove = playTurn(board, &rootIndex, ((rand() / (RAND_MAX * 2.0)) + 0.75) * TIME, previousMove);
        } else {
            previousMove = playTurnOpponent(&stateOpponent, ((rand() / (RAND_MAX * 2.0)) + 0.75) * TIME, previousMove);
        }
        weAreCurrentPlayer = !weAreCurrentPlayer;
    }
    if (board->state.winner == NONE) {
        makePermanentMove(board, previousMove);
    }
    Winner winner = board->state.winner;
    assert(winner != NONE && "simulateSingleGame: oops");
    freeStateOpponent(stateOpponent);
    freeBoard(board);
    return winner;
}


void runArena() {
    srand(69);
    int winsGoingFirst = 0;
    int drawsGoingFirst = 0;
    int lossesGoingFirst = 0;
    int winsGoingSecond = 0;
    int drawsGoingSecond = 0;
    int lossesGoingSecond = 0;
    #pragma omp parallel for default(none) shared(winsGoingFirst, winsGoingSecond, drawsGoingFirst, drawsGoingSecond, lossesGoingFirst, lossesGoingSecond)
    for (int i = 0; i < ROUNDS/2; i++) {
        Winner winner = simulateSingleGame(true);
        if (winner == WIN_P1) {
            winsGoingFirst++;
        } else if (winner == WIN_P2) {
            lossesGoingFirst++;
        } else {
            drawsGoingFirst++;
        }
        winner = simulateSingleGame(false);
        if (winner == WIN_P2) {
            winsGoingSecond++;
        } else if (winner == WIN_P1) {
            lossesGoingSecond++;
        } else {
            drawsGoingSecond++;
        }
    }
    double denominator = ROUNDS / 2.0;
    printf("Going first:\n");
    printf("\tWon %.2f%% of games\n", 100*winsGoingFirst / denominator);
    printf("\tDrew %.2f%% of games\n", 100*drawsGoingFirst / denominator);
    printf("\tLost %.2f%% of games\n", 100*lossesGoingFirst / denominator);
    printf("Going second:\n");
    printf("\tWon %.2f%% of games\n", 100*winsGoingSecond / denominator);
    printf("\tDrew %.2f%% of games\n", 100*drawsGoingSecond / denominator);
    printf("\tLost %.2f%% of games\n", 100*lossesGoingSecond / denominator);
    printf("Total score: %f\n", (winsGoingFirst+winsGoingSecond + drawsGoingFirst/2.+drawsGoingSecond/2.) / (double)ROUNDS);
}