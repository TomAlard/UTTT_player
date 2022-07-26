#include <stdio.h>
#include "mcts_node_tests.h"
#include "../src/mcts_node.h"
#include "test_util.h"


void rootIsLeafNode() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    myAssert(isLeafNode(root, board));
    freeBoard(board);
    freeMCTSTree(root);
}


void between17And81MovesInOneGame() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    MCTSNode* node = root;
    int count = 0;
    while (isLeafNode(node, board)) {
        node = selectNextChild(node);
        visitNode(node, board);
        count++;
    }
    myAssert(count >= 17 && count <= 81);
    freeBoard(board);
    freeMCTSTree(root);
}


void keepsSelectingSameNodeWhenSetAsWin() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    isLeafNode(root, board);  // to discover child nodes
    MCTSNode* node = selectNextChild(root);
    setNodeWinner(node, WIN_P1);
    MCTSNode* untriedNode = selectNextChild(root);
    while (getSims(untriedNode) == 0) {
        // backpropagate 100 wins
        for (int i = 0; i < 100; i++) {
            backpropagate(untriedNode, WIN_P1);
        }
        untriedNode = selectNextChild(root);
    }
    myAssert(selectNextChild(root) == node);
    freeBoard(board);
    freeMCTSTree(root);
}


void selectsChildWithHighChanceOfWin() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    isLeafNode(root, board);  // to discover child nodes
    MCTSNode* winNode = selectNextChild(root);
    // backpropagate one loss and 99 wins
    backpropagate(winNode, WIN_P2);
    for (int i = 0; i < 99 ; i++) {
        backpropagate(winNode, WIN_P1);
    }
    MCTSNode* node = selectNextChild(root);
    while (getSims(node) == 0) {
        // backpropagate one win and 99 losses
        backpropagate(node, WIN_P1);
        for (int i = 0; i < 99; i++) {
            backpropagate(node, WIN_P2);
        }
        node = selectNextChild(root);
    }
    myAssert(node == winNode);
    freeBoard(board);
    freeMCTSTree(root);
}


void updateRootTest() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    isLeafNode(root, board);  // to discover child nodes
    backpropagate(selectNextChild(root), DRAW);
    Square square = getMostPromisingMove(root);
    MCTSNode* newRoot = updateRoot(root, board, square);
    isLeafNode(newRoot, board);  // to discover child nodes
    backpropagate(selectNextChild(newRoot), DRAW);
    myAssert(getMostPromisingMove(newRoot).board == square.position);
    freeBoard(board);
    freeMCTSTree(newRoot);
}


void alwaysPlays44WhenGoingFirst() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    setMe(board, PLAYER1);
    isLeafNode(root, board);  // to discover child nodes
    selectNextChild(root);  // to expand at least one child
    Square expected = {4, 4};
    myAssert(squaresAreEqual(getMostPromisingMove(root), expected));
    freeBoard(board);
    freeMCTSTree(root);
}


void runMCTSNodeTests() {
    printf("\trootIsLeafNode...\n");
    rootIsLeafNode();
    printf("\tbetween17And81MovesInOneGame...\n");
    between17And81MovesInOneGame();
    printf("\tkeepsSelectingSameNodeWhenSetAsWin...\n");
    keepsSelectingSameNodeWhenSetAsWin();
    printf("\tselectsChildWithHighChanceOfWin...\n");
    selectsChildWithHighChanceOfWin();
    printf("\tupdateRootTest...\n");
    updateRootTest();
    printf("\talwaysPlays44WhenGoingFirst...\n");
    alwaysPlays44WhenGoingFirst();
}