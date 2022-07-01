#include <stdio.h>
#include "mcts_node_tests.h"
#include "../src/mcts_node.h"
#include "test_util.h"


void rootHasChildrenAndIsLeaf() {
    MCTSNode* root = createMCTSRootNode();
    BitBoard* board = createBitBoard();
    myAssert(hasChildren(root, board) && isLeafNode(root));
    freeBitBoard(board);
    freeMCTSTree(root);
}


void between17And81MovesInOneGame() {
    MCTSNode* root = createMCTSRootNode();
    BitBoard* board = createBitBoard();
    MCTSNode* node = root;
    int count = 0;
    while (hasChildren(node, board)) {
        node = selectNextChild(node, board);
        visitNode(node, board);
        count++;
    }
    myAssert(count >= 17 && count <= 81);
    freeBitBoard(board);
    freeMCTSTree(root);
}


void keepsSelectingSameNodeWhenSetAsWin() {
    MCTSNode* root = createMCTSRootNode();
    BitBoard* board = createBitBoard();
    MCTSNode* node = selectNextChild(root, board);
    setNodeWinner(node, WIN_P2);
    myAssert(selectNextChild(root, board) == node);
    freeBitBoard(board);
    freeMCTSTree(root);
}


void selectsChildWithHighChanceOfWin() {
    MCTSNode* root = createMCTSRootNode();
    BitBoard* board = createBitBoard();
    MCTSNode* winNode = selectNextChild(root, board);
    // backpropagate one loss and 99 wins
    backpropagate(winNode, WIN_P1);
    for (int i = 0; i < 99 ; i++) {
        backpropagate(winNode, WIN_P2);
    }
    MCTSNode* node = selectNextChild(root, board);
    while (isLeafNode(node)) {
        // backpropagate one win and 99 losses
        backpropagate(node, WIN_P2);
        for (int i = 0; i < 99; i++) {
            backpropagate(node, WIN_P1);
        }
        node = selectNextChild(root, board);
    }
    myAssert(node == winNode);
    freeBitBoard(board);
    freeMCTSTree(root);
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
}