#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <sys/time.h>
#include "../misc/player.h"
#include "../main.h"
#include "arena_opponent.h"
#include "../misc/util.h"


#define ROUNDS 100
#define TIME 0.1


Winner simulateSingleGame(bool weArePlayer1, int game) {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode();
    RNG rng;
    seedRNG(&rng, time(NULL), 420);
    StateOpponent* stateOpponent = initializeStateOpponent();
    Square previousMove = {9, 9};
    bool weAreCurrentPlayer = weArePlayer1;
    char buffer[1000];
    snprintf(buffer, 1000, "./src/arena/games/delet_game_%d.csv", game);
    FILE* file = fopen(buffer, "w");
    while (getWinner(board) == NONE && stateOpponent != NULL) {
        float winrate;
        State state;
        struct timeval start, end;
        gettimeofday(&start, NULL);
        if (weAreCurrentPlayer) {
            if (previousMove.board != 9) {
                makeTemporaryMove(board, previousMove);
                state = board->state;
                revertToCheckpoint(board);
            } else {
                state = board->state;
            }
            previousMove = playTurn(board, &root, &rng, TIME, previousMove);
            winrate = getWinrate(root);
        } else {
            state = board->state;
            previousMove = playTurnOpponent(&stateOpponent, TIME, previousMove);
            winrate = stateOpponent != NULL? getWinrateOpponent(stateOpponent) : 0.0f;
        }
        gettimeofday(&end, NULL);
        double timePassed = (double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec);
        if (timePassed > TIME*2) {
            fprintf(stderr, weAreCurrentPlayer? "We used too much time!\n" : "Opponent used too much time!\n");
        }
        char p1SmallBoardBits[81];
        char p2SmallBoardBits[81];
        for (int j = 0; j < 81; j++) {
            p1SmallBoardBits[j] = (char)((BIT_CHECK(state.player1.smallBoards[j/9], j%9) != 0) + '0');
            p2SmallBoardBits[j] = (char)((BIT_CHECK(state.player2.smallBoards[j/9], j%9) != 0) + '0');
        }
        char p1BigBoardBits[9];
        char p2BigBoardBits[9];
        for (int j = 0; j < 9; j++) {
            p1BigBoardBits[j] = (char)((BIT_CHECK(state.player1.bigBoard, j) != 0) + '0');
            p2BigBoardBits[j] = (char)((BIT_CHECK(state.player2.bigBoard, j) != 0) + '0');
        }
        fprintf(file, "%.*s,%.*s,%.*s,%.*s,%d,%d,%.4f,%d%d\n", 81, p1SmallBoardBits, 81, p2SmallBoardBits,
                9, p1BigBoardBits, 9, p2BigBoardBits, state.currentPlayer, state.currentBoard, winrate, previousMove.board, previousMove.position);
        weAreCurrentPlayer = !weAreCurrentPlayer;
    }
    if (getWinner(board) == NONE) {
        makePermanentMove(board, previousMove);
    }
    Winner winner = getWinner(board);
    assert(winner != NONE && "simulateSingleGame: oops");
    fprintf(file, "%c\n\n", winner + '0');
    fclose(file);
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
    // #pragma omp parallel for default(none) shared(winsGoingFirst, winsGoingSecond, drawsGoingFirst, drawsGoingSecond, lossesGoingFirst, lossesGoingSecond)
    for (int i = 0; i < ROUNDS/2; i++) {
        Winner winner = simulateSingleGame(true, i*2);
        if (winner == WIN_P1) {
            winsGoingFirst++;
        } else if (winner == WIN_P2) {
            lossesGoingFirst++;
        } else {
            drawsGoingFirst++;
        }
        winner = simulateSingleGame(false, i*2 + 1);
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