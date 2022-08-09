#include <stdio.h>
#include "mcts_node_tests.h"
#include "../../src/mcts/mcts_node.h"
#include "../test_util.h"


void rootIsLeafNode() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    RNG rng;
    seedRNG(&rng, 69, 420);
    myAssert(isLeafNode(root, board, &rng));
    freeBoard(board);
    freeMCTSTree(root);
}


void between17And81MovesInOneGame() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    RNG rng;
    seedRNG(&rng, 69, 420);
    MCTSNode* node = root;
    int count = 0;
    while (isLeafNode(node, board, &rng)) {
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
    RNG rng;
    seedRNG(&rng, 69, 420);
    isLeafNode(root, board, &rng);  // to discover child nodes
    MCTSNode* winNode = expandNextChild(root);
    // backpropagate one loss and 99 wins
    backpropagate(winNode, WIN_P2, getCurrentPlayer(board));
    for (int i = 0; i < 99 ; i++) {
        backpropagate(winNode, WIN_P1, getCurrentPlayer(board));
    }
    MCTSNode* node = expandNextChild(root);
    while (isLeafNode(root, board, &rng)) {
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
    RNG rng;
    seedRNG(&rng, 69, 420);
    isLeafNode(root, board, &rng);  // to discover child nodes
    backpropagate(expandNextChild(root), DRAW, getCurrentPlayer(board));
    Square square = getMostPromisingMove(root);
    makePermanentMove(board, square);
    MCTSNode* newRoot = updateRoot(root, board, square);
    isLeafNode(newRoot, board, &rng);  // to discover child nodes
    backpropagate(expandNextChild(newRoot), DRAW, getCurrentPlayer(board));
    myAssert(getMostPromisingMove(newRoot).board == square.position);
    freeBoard(board);
    freeMCTSTree(newRoot);
}


void updateRootStillWorksWhenPlayedMoveWasPruned() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    RNG rng;
    seedRNG(&rng, 69, 420);
    Square firstMove = {4, 4};
    isLeafNode(root, board, &rng);  // to discover child nodes
    root = updateRoot(root, board, firstMove);
    makePermanentMove(board, firstMove);
    isLeafNode(root, board, &rng);  // to discover child nodes
    root->amountOfUntriedMoves = 1;  // 'prune' the other 8 moves
    Square prunedMove = {4, 5};
    myAssert(!squaresAreEqual(expandNextChild(root)->square, prunedMove));
    root = updateRoot(root, board, prunedMove);
    freeBoard(board);
    freeMCTSTree(root);
}


void alwaysPlays44WhenGoingFirst() {
    MCTSNode* root = createMCTSRootNode();
    Board* board = createBoard();
    RNG rng;
    seedRNG(&rng, 69, 420);
    setMe(board, PLAYER1);
    isLeafNode(root, board, &rng);  // to discover child nodes
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
    printf("\tupdateRootStillWorksWhenPlayedMoveWasPruned...\n");
    updateRootStillWorksWhenPlayedMoveWasPruned();
    printf("\talwaysPlays44WhenGoingFirst...\n");
    alwaysPlays44WhenGoingFirst();
}