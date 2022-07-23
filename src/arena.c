#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include "player.h"
#include "main.h"
#include "arena_opponent.h"
#include "util.h"
#include "handle_turn.h"


#define ROUNDS 100
#define TIME 0.001


Winner simulateSingleGame(bool weArePlayer1) {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode();
    pcg32_random_t rng;
    pcg32_srandom_r(&rng, time(NULL), 420);
    StateOpponent* stateOpponent = initializeStateOpponent();
    Square previousMove = {9, 9};
    bool weAreCurrentPlayer = weArePlayer1;
    while (getWinner(board) == NONE && stateOpponent != NULL) {
        if (weAreCurrentPlayer) {
            previousMove = playTurn(board, &root, &rng, TIME, previousMove);
        } else {
            previousMove = playTurnOpponent(&stateOpponent, TIME, previousMove);
        }
        weAreCurrentPlayer = !weAreCurrentPlayer;
    }
    if (getWinner(board) == NONE) {
        makePermanentMove(board, previousMove);
    }
    Winner winner = getWinner(board);
    assert(winner != NONE && "simulateSingleGame: oops");
    freeStateOpponent(stateOpponent);
    freeMCTSTree(root);
    freeBoard(board);
    return winner;
}


void runArena() {
    int winsGoingFirst = 0;
    int drawsGoingFirst = 0;
    int lossesGoingFirst = 0;
    int winsGoingSecond = 0;
    int drawsGoingSecond = 0;
    int lossesGoingSecond = 0;
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
}