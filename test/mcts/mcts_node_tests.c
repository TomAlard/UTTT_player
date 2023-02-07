#include <stdio.h>
#include <math.h>
#include "mcts_node_tests.h"
#include "../../src/mcts/mcts_node.h"
#include "../test_util.h"
#include "../../src/nn/forward.h"


void rootIsLeafNode() {
    Board* board = createBoard();
    int rootIndex = createMCTSRootNode(board);
    myAssert(isLeafNode(rootIndex, board));
    freeBoard(board);
}


void between17And81MovesInOneGame() {
    Board* board = createBoard();
    int nodeIndex = createMCTSRootNode(board);
    discoverChildNodes(nodeIndex, board);
    int count = 0;
    MCTSNode* node = &board->nodes[nodeIndex];
    while (node->amountOfChildren > 0) {
        nodeIndex = selectNextChild(board, nodeIndex);
        node = &board->nodes[nodeIndex];
        visitNode(nodeIndex, board);
        discoverChildNodes(nodeIndex, board);
        count++;
    }
    myAssert(count >= 17 && count <= 81);
    freeBoard(board);
}


void updateRootTest() {
    Board* board = createBoard();
    int rootIndex = createMCTSRootNode(board);
    MCTSNode* root = &board->nodes[rootIndex];
    discoverChildNodes(rootIndex, board);
    backpropagate(board, root->childrenIndex + 0, DRAW, getCurrentPlayer(board));
    Square square = getMostPromisingMove(board, root);
    makePermanentMove(board, square);
    int newRootIndex = updateRoot(root, board, square);
    MCTSNode* newRoot = &board->nodes[newRootIndex];
    discoverChildNodes(newRootIndex, board);
    backpropagate(board, newRoot->childrenIndex + 0, DRAW, getCurrentPlayer(board));
    myAssert(getMostPromisingMove(board, newRoot).board == square.position);
    freeBoard(board);
}


void updateRootStillWorksWhenPlayedMoveWasPruned() {
    Board* board = createBoard();
    int rootIndex = createMCTSRootNode(board);
    MCTSNode* root = &board->nodes[rootIndex];
    Square firstMove = {4, 4};
    discoverChildNodes(rootIndex, board);
    rootIndex = updateRoot(root, board, firstMove);
    root = &board->nodes[rootIndex];
    makePermanentMove(board, firstMove);
    discoverChildNodes(rootIndex, board);
    root->amountOfChildren = 1;  // 'prune' the other 8 moves
    Square prunedMove = {4, 5};
    myAssert(!squaresAreEqual((&board->nodes[root->childrenIndex + 0])->square, prunedMove));
    updateRoot(root, board, prunedMove);
    freeBoard(board);
}


void alwaysPlays44WhenGoingFirst() {
    Board* board = createBoard();
    int rootIndex = createMCTSRootNode(board);
    setMe(board, PLAYER1);
    discoverChildNodes(rootIndex, board);
    Square expected = {4, 4};
    myAssert(squaresAreEqual(getMostPromisingMove(board, &board->nodes[rootIndex]), expected));
    freeBoard(board);
}


void optimizedNNEvalTest() {
    Board* board = createBoard();
    int rootIndex = createMCTSRootNode(board);
    MCTSNode* root = &board->nodes[rootIndex];
    discoverChildNodes(rootIndex, board);
    for (int i = 0; i < root->amountOfChildren; i++) {
        MCTSNode* child = &board->nodes[root->childrenIndex + i];
        makeTemporaryMove(board, child->square);
        myAssert(fabsf(neuralNetworkEval(board) - child->eval) < 1e-5);
        revertToCheckpoint(board);
    }
    freeBoard(board);
}


void runMCTSNodeTests() {
    printf("\trootIsLeafNode...\n");
    rootIsLeafNode();
    printf("\tbetween17And81MovesInOneGame...\n");
    between17And81MovesInOneGame();
    printf("\tupdateRootTest...\n");
    updateRootTest();
    printf("\tupdateRootStillWorksWhenPlayedMoveWasPruned...\n");
    updateRootStillWorksWhenPlayedMoveWasPruned();
    printf("\talwaysPlays44WhenGoingFirst...\n");
    alwaysPlays44WhenGoingFirst();
    printf("\toptimizedNNEvalTest...\n");
    optimizedNNEvalTest();
}