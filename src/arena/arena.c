#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <sys/time.h>
#include "../misc/player.h"
#include "../main.h"
#include "arena_opponent.h"
#include "../misc/util.h"


#define ROUNDS 1000
#define TIME 0.1


Winner simulateSingleGame(bool weArePlayer1) {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode();
    RNG rng;
    seedRNG(&rng, time(NULL), 420);
    StateOpponent* stateOpponent = initializeStateOpponent();
    Square previousMove = {9, 9};
    bool weAreCurrentPlayer = weArePlayer1;
    while (getWinner(board) == NONE && stateOpponent != NULL) {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        if (weAreCurrentPlayer) {
            previousMove = playTurn(board, &root, &rng, TIME, previousMove);
        } else {
            previousMove = playTurnOpponent(&stateOpponent, TIME, previousMove);
        }
        gettimeofday(&end, NULL);
        double timePassed = (double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec);
        if (timePassed > TIME*2) {
            fprintf(stderr, weAreCurrentPlayer? "We used too much time!\n" : "Opponent used too much time!\n");
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