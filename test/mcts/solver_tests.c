#include <stdio.h>
#include "solver_tests.h"
#include "../../src/mcts/solver.h"
#include "../test_util.h"


void keepsSelectingSameNodeWhenSetAsWin() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    isLeafNode(root, board);  // to discover child nodes
    MCTSNode* node = selectNextChild(root);
    setNodeWinner(node, WIN_P1, getCurrentPlayer(board));
    MCTSNode* untriedNode = selectNextChild(root);
    while (getSims(untriedNode) == 0) {
        // backpropagate 100 wins
        for (int i = 0; i < 100; i++) {
            backpropagate(untriedNode, WIN_P1, getCurrentPlayer(board));
        }
        untriedNode = selectNextChild(root);
    }
    myAssert(selectNextChild(root) == node);
    freeBoard(board);
    freeMCTSTree(root);
}


void parentIsWinIfOneChildIsSetAsLoss() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    isLeafNode(root, board);  // to discover child nodes
    setNodeWinner(selectNextChild(root), WIN_P2, PLAYER1);
    myAssert(root->wins == root->sims && root->wins > 0);
    freeBoard(board);
    freeMCTSTree(root);
}


void parentIsLossIfAllChildrenAreSetAsWin() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    while (isLeafNode(root, board)) {
        setNodeWinner(selectNextChild(root), WIN_P1, PLAYER1);
    }
    myAssert(root->wins == 0 && root->sims > 0);
    freeBoard(board);
    freeMCTSTree(root);
}


void parentIsDrawIfAtLeastOneChildIsDrawAndOthersAreWin() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    isLeafNode(root, board);  // to discover child nodes
    setNodeWinner(selectNextChild(root), DRAW, PLAYER1);
    while (isLeafNode(root, board)) {
        setNodeWinner(selectNextChild(root), WIN_P1, PLAYER1);
    }
    myAssert(root->wins / root->sims == 0.5f);
    freeBoard(board);
    freeMCTSTree(root);
}


void runSolverTests() {
    printf("\tkeepsSelectingSameNodeWhenSetAsWin...\n");
    keepsSelectingSameNodeWhenSetAsWin();
    printf("\tparentIsWinIfChildIsSetAsLoss...\n");
    parentIsWinIfOneChildIsSetAsLoss();
    printf("\tparentIsLossIfAllChildrenAreSetAsWin...\n");
    parentIsLossIfAllChildrenAreSetAsWin();
    printf("\tparentIsDrawIfAtLeastOneChildIsDrawAndOthersAreWin...\n");
    parentIsDrawIfAtLeastOneChildIsDrawAndOthersAreWin();
}