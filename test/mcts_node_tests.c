#include <stdio.h>
#include "mcts_node_tests.h"
#include "../src/mcts_node.h"
#include "test_util.h"


void rootHasChildrenAndIsLeaf() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    myAssert(hasChildren(root, board) && isLeafNode(root));
    freeBoard(board);
    freeMCTSTree(root);
}


void between17And81MovesInOneGame() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    MCTSNode* node = root;
    int count = 0;
    while (hasChildren(node, board)) {
        node = selectNextChild(node, board);
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
    MCTSNode* node = selectNextChild(root, board);
    setNodeWinner(node, WIN_P1);
    myAssert(selectNextChild(root, board) == node);
    freeBoard(board);
    freeMCTSTree(root);
}


void selectsChildWithHighChanceOfWin() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    MCTSNode* winNode = selectNextChild(root, board);
    // backpropagate one loss and 99 wins
    backpropagate(winNode, WIN_P2);
    for (int i = 0; i < 99 ; i++) {
        backpropagate(winNode, WIN_P1);
    }
    MCTSNode* node = selectNextChild(root, board);
    while (isLeafNode(node)) {
        // backpropagate one win and 99 losses
        backpropagate(node, WIN_P1);
        for (int i = 0; i < 99; i++) {
            backpropagate(node, WIN_P2);
        }
        node = selectNextChild(root, board);
    }
    myAssert(node == winNode);
    freeBoard(board);
    freeMCTSTree(root);
}


void updateRootTest() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    hasChildren(root, board);  // discover children
    Square square = getMostSimulatedChildSquare(root);
    MCTSNode* newRoot = updateRoot(root, board, square);
    hasChildren(newRoot, board);  // discover children
    myAssert(getMostSimulatedChildSquare(newRoot).board == square.position);
    freeBoard(board);
    freeMCTSTree(newRoot);
}


void runMCTSNodeTests() {
    printf("\trootHasChildrenAndIsLeaf...\n");
    rootHasChildrenAndIsLeaf();
    printf("\tbetween17And81MovesInOneGame...\n");
    between17And81MovesInOneGame();
    printf("\tkeepsSelectingSameNodeWhenSetAsWin...\n");
    keepsSelectingSameNodeWhenSetAsWin();
    printf("\tselectsChildWithHighChanceOfWin...\n");
    selectsChildWithHighChanceOfWin();
    printf("\tupdateRootTest...\n");
    updateRootTest();
}