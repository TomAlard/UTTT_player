#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mcts_node.h"
#include "util.h"


typedef struct MCTSNode {
    MCTSNode* parent;
    MCTSNode* children;
    float wins;
    float sims;
    Square square;
    int8_t amountOfChildren;
    int8_t amountOfUntriedMoves;
} MCTSNode;


MCTSNode* createMCTSRootNode() {
    MCTSNode* root = safe_calloc(sizeof(MCTSNode));
    root->amountOfChildren = -1;
    root->square.board = 9;
    root->square.position = 9;
    root->amountOfUntriedMoves = -1;
    return root;
}


void initializeMCTSNode(MCTSNode* parent, Square square, MCTSNode* node) {
    node->parent = parent;
    node->children = NULL;
    node->wins = 0.0f;
    node->sims = 0.0f;
    node->square = square;
    node->amountOfChildren = -1;
    node->amountOfUntriedMoves = -1;
}


MCTSNode* copyMCTSNode(MCTSNode* original) {
    MCTSNode* copy = safe_malloc(sizeof(MCTSNode));
    copy->parent = original->parent;
    copy->children = original->children;
    copy->wins = original->wins;
    copy->sims = original->sims;
    copy->square = original->square;
    copy->amountOfChildren = original->amountOfChildren;
    copy->amountOfUntriedMoves = original->amountOfUntriedMoves;
    for (int i = 0; i < copy->amountOfChildren; i++) {
        copy->children[i].parent = copy;
    }
    return copy;
}


void freeMCTSTree(MCTSNode* node) {
    for (int i = 0; i < node->amountOfChildren; i++) {
        freeMCTSTree(&node->children[i]);
    }
    free(node->children);
}


void singleChild(MCTSNode* node, Square square) {
    node->amountOfUntriedMoves = 1;
    node->children = safe_malloc(sizeof(MCTSNode));
    node->children[0].square = square;
}


bool handleSpecialCases(MCTSNode* node, Board* board) {
    if (nextBoardIsEmpty(board) && currentPlayerIsMe(board) && getPly(board) <= 20) {
        uint8_t currentBoard = getCurrentBoard(board);
        Square sameBoard = {currentBoard, currentBoard};
        singleChild(node, sameBoard);
        return true;
    }
    if (getPly(board) == 0) {
        Square bestFirstMove = {4, 4};
        singleChild(node, bestFirstMove);
        return true;
    }
    return false;
}


void discoverChildNodes(MCTSNode* node, Board* board) {
    if (node->amountOfChildren == -1) {
        node->amountOfChildren = 0;
        if (!handleSpecialCases(node, board)) {
            Square movesArray[TOTAL_SMALL_SQUARES];
            int8_t amountOfMoves;
            Square* moves = generateMoves(board, movesArray, &amountOfMoves);
            node->amountOfUntriedMoves = amountOfMoves;
            node->children = safe_malloc(amountOfMoves * sizeof(MCTSNode));
            for (int i = 0; i < amountOfMoves; i++) {
                node->children[i].square = moves[i];
            }
        }
    }
}


bool isLeafNode(MCTSNode* node, Board* board) {
    discoverChildNodes(node, board);
    return node->amountOfUntriedMoves > 0;
}


#define EXPLORATION_PARAMETER 0.459375f
float getUCTValue(MCTSNode* node, float parentLogSims) {
    float w = node->wins;
    float n = node->sims;
    float c = EXPLORATION_PARAMETER;
    return w/n + c*sqrtf(parentLogSims / n);
}


MCTSNode* expandNode(MCTSNode* node, int childIndex) {
    MCTSNode* newChild = &node->children[childIndex + node->amountOfChildren++];
    initializeMCTSNode(node, newChild->square, newChild);
    node->amountOfUntriedMoves--;
    return newChild;
}


MCTSNode* selectNextChild(MCTSNode* node) {
    assert(node->amountOfUntriedMoves > 0 || node->amountOfChildren > 0 && "selectNextChild: node is terminal");
    if (node->amountOfUntriedMoves) {
        return expandNode(node, 0);
    }
    float logSims = logf(node->sims);
    MCTSNode* highestUCTChild = &node->children[0];
    float highestUCT = getUCTValue(highestUCTChild, logSims);
    for (int i = 1; i < node->amountOfChildren; i++) {
        MCTSNode* child = &node->children[i];
        float UCT = getUCTValue(child, logSims);
        if (UCT > highestUCT) {
            highestUCTChild = child;
            highestUCT = UCT;
        }
    }
    assert(highestUCTChild != NULL && "selectNextChild: Panic! This should be impossible.");
    return highestUCTChild;
}


MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square) {
    discoverChildNodes(root, board);
    assert(root->amountOfUntriedMoves > 0 || root->amountOfChildren > 0 && "updateRoot: root is terminal");
    MCTSNode* newRoot = NULL;
    for (int i = 0; i < root->amountOfChildren; i++) {
        MCTSNode* child = &root->children[i];
        if (squaresAreEqual(square, child->square)) {
            assert(newRoot == NULL && "updateRoot: multiple children with same square found");
            newRoot = child;
        } else {
            freeMCTSTree(child);
        }
    }
    if (newRoot == NULL) {
        for (int i = 0; i < root->amountOfUntriedMoves; i++) {
            if (squaresAreEqual(square, root->children[root->amountOfChildren + i].square)) {
                newRoot = expandNode(root, i);
                break;
            }
        }
    }
    assert(newRoot != NULL && "updateRoot: newRoot shouldn't be NULL");
    newRoot->parent = NULL;
    newRoot = copyMCTSNode(newRoot);
    free(root->children);
    return newRoot;
}


void backpropagate(MCTSNode* node, Winner winner, Player player) {
    assert(winner != NONE && "backpropagate: Can't backpropagate a NONE Winner");
    MCTSNode* currentNode = node;
    float reward = winner == DRAW? 0.5f : player + 1 == winner? 1.0f : 0.0f;
    while (currentNode != NULL) {
        currentNode->sims++;
        currentNode->wins += reward;
        reward = 1 - reward;
        currentNode = currentNode->parent;
    }
}


void visitNode(MCTSNode* node, Board* board) {
    makeTemporaryMove(board, node->square);
}


#define A_LOT 100000.0f
void setNodeWinner(MCTSNode* node, Winner winner, Player player) {
    assert(winner != NONE && "setNodeWinner: Can't set NONE as winner");
    if (winner != DRAW) {
        bool win = player + 1 == winner;
        node->wins += win? A_LOT : -A_LOT;
        if (!win) {
            node->parent->wins += A_LOT;
        }
    }
}


Square getMostPromisingMove(MCTSNode* node) {
    assert(node->amountOfChildren > 0 && "getMostPromisingMove: node has no children");
    MCTSNode* highestSimsChild = &node->children[0];
    float highestSims = highestSimsChild->sims;
    for (int i = 1; i < node->amountOfChildren; i++) {
        MCTSNode* child = &node->children[i];
        float sims = child->sims;
        if (sims > highestSims) {
            highestSimsChild = child;
            highestSims = sims;
        }
    }
    return highestSimsChild->square;
}


int getSims(MCTSNode* node) {
    return (int) node->sims;
}


float getWinrate(MCTSNode* node) {
    return node->wins / node->sims;
}
