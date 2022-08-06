#include <stdio.h>
#include "solver_tests.h"
#include "../../src/mcts/solver.h"
#include "../test_util.h"


void parentIsLossIfOneChildIsSetAsWin() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    isLeafNode(root, board);  // to discover child nodes
    setNodeWinner(expandNextChild(root), WIN_P2, PLAYER2);
    myAssert(root->wins == 0 && root->sims > 0);
    freeBoard(board);
    freeMCTSTree(root);
}


void parentIsWinIfAllChildrenAreSetAsLoss() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    while (isLeafNode(root, board)) {
        setNodeWinner(expandNextChild(root), WIN_P2, PLAYER1);
    }
    myAssert(root->wins == root->sims && root->wins > 0);
    freeBoard(board);
    freeMCTSTree(root);
}


void parentIsDrawIfAtLeastOneChildIsDrawAndOthersAreLoss() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    isLeafNode(root, board);  // to discover child nodes
    setNodeWinner(expandNextChild(root), DRAW, PLAYER1);
    while (isLeafNode(root, board)) {
        setNodeWinner(expandNextChild(root), WIN_P2, PLAYER1);
    }
    myAssert(root->wins / root->sims == 0.5f);
    freeBoard(board);
    freeMCTSTree(root);
}


void parentIsLossIfAllGrandchildrenOfOneChildAreLoss() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    isLeafNode(root, board);  // to discover child nodes
    MCTSNode* child = expandNextChild(root);
    while (isLeafNode(child, board)) {
        setNodeWinner(expandNextChild(child), WIN_P1, PLAYER2);
    }
    myAssert(root->wins == 0 && root->sims > 0);
    freeBoard(board);
    freeMCTSTree(root);
}


void runSolverTests() {
    printf("\tparentIsLossIfOneChildIsSetAsWin...\n");
    parentIsLossIfOneChildIsSetAsWin();
    printf("\tparentIsWinIfAllChildrenAreSetAsLoss...\n");
    parentIsWinIfAllChildrenAreSetAsLoss();
    printf("\tparentIsDrawIfAtLeastOneChildIsDrawAndOthersAreLoss...\n");
    parentIsDrawIfAtLeastOneChildIsDrawAndOthersAreLoss();
    printf("\tparentIsLossIfAllGrandchildrenOfOneChildAreLoss...\n");
    parentIsLossIfAllGrandchildrenOfOneChildAreLoss();
}