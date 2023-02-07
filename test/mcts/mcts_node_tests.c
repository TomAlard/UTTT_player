#include <stdio.h>
#include <math.h>
#include "mcts_node_tests.h"
#include "../../src/mcts/mcts_node.h"
#include "../test_util.h"
#include "../../src/nn/forward.h"


void rootIsLeafNode() {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode(board);
    myAssert(isLeafNode(root));
    freeBoard(board);
}


void between17And81MovesInOneGame() {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode(board);
    MCTSNode* node = root;
    discoverChildNodes(node, board);
    int count = 0;
    while (node->amountOfChildren > 0) {
        node = selectNextChild(node);
        visitNode(node, board);
        discoverChildNodes(node, board);
        count++;
    }
    myAssert(count >= 17 && count <= 81);
    freeBoard(board);
}


void updateRootTest() {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode(board);
    discoverChildNodes(root, board);
    backpropagate(&root->children[0], DRAW, getCurrentPlayer(board));
    Square square = getMostPromisingMove(root);
    makePermanentMove(board, square);
    MCTSNode* newRoot = updateRoot(root, board, square);
    discoverChildNodes(newRoot, board);
    backpropagate(&newRoot->children[0], DRAW, getCurrentPlayer(board));
    myAssert(getMostPromisingMove(newRoot).board == square.position);
    freeBoard(board);
}


void updateRootStillWorksWhenPlayedMoveWasPruned() {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode(board);
    Square firstMove = {4, 4};
    discoverChildNodes(root, board);
    root = updateRoot(root, board, firstMove);
    makePermanentMove(board, firstMove);
    discoverChildNodes(root, board);
    root->amountOfChildren = 1;  // 'prune' the other 8 moves
    Square prunedMove = {4, 5};
    myAssert(!squaresAreEqual(root->children[0].square, prunedMove));
    updateRoot(root, board, prunedMove);
    freeBoard(board);
}


void alwaysPlays44WhenGoingFirst() {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode(board);
    setMe(board, PLAYER1);
    discoverChildNodes(root, board);
    Square expected = {4, 4};
    myAssert(squaresAreEqual(getMostPromisingMove(root), expected));
    freeBoard(board);
}


void optimizedNNEvalTest() {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode(board);
    discoverChildNodes(root, board);
    for (int i = 0; i < root->amountOfChildren; i++) {
        MCTSNode* child = &root->children[i];
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