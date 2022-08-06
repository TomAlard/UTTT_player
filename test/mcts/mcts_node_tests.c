#include <stdio.h>
#include "mcts_node_tests.h"
#include "../../src/mcts/mcts_node.h"
#include "../test_util.h"


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
        node = expandNextChild(node);
        visitNode(node, board);
        count++;
    }
    myAssert(count >= 17 && count <= 81);
    freeBoard(board);
    freeMCTSTree(root);
}


void selectsChildWithHighChanceOfWin() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    isLeafNode(root, board);  // to discover child nodes
    MCTSNode* winNode = expandNextChild(root);
    // backpropagate one loss and 99 wins
    backpropagate(winNode, WIN_P2, getCurrentPlayer(board));
    for (int i = 0; i < 99 ; i++) {
        backpropagate(winNode, WIN_P1, getCurrentPlayer(board));
    }
    MCTSNode* node = expandNextChild(root);
    while (isLeafNode(root, board)) {
        // backpropagate one win and 99 losses
        backpropagate(node, WIN_P1, getCurrentPlayer(board));
        for (int i = 0; i < 99; i++) {
            backpropagate(node, WIN_P2, getCurrentPlayer(board));
        }
        node = expandNextChild(root);
    }
    myAssert(selectNextChild(root) == winNode);
    freeBoard(board);
    freeMCTSTree(root);
}


void updateRootTest() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    isLeafNode(root, board);  // to discover child nodes
    backpropagate(expandNextChild(root), DRAW, getCurrentPlayer(board));
    Square square = getMostPromisingMove(root);
    makePermanentMove(board, square);
    MCTSNode* newRoot = updateRoot(root, board, square);
    isLeafNode(newRoot, board);  // to discover child nodes
    backpropagate(expandNextChild(newRoot), DRAW, getCurrentPlayer(board));
    myAssert(getMostPromisingMove(newRoot).board == square.position);
    freeBoard(board);
    freeMCTSTree(newRoot);
}


void alwaysPlays44WhenGoingFirst() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    setMe(board, PLAYER1);
    isLeafNode(root, board);  // to discover child nodes
    expandNextChild(root);  // to expand at least one child
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
    printf("\tselectsChildWithHighChanceOfWin...\n");
    selectsChildWithHighChanceOfWin();
    printf("\tupdateRootTest...\n");
    updateRootTest();
    printf("\talwaysPlays44WhenGoingFirst...\n");
    alwaysPlays44WhenGoingFirst();
}