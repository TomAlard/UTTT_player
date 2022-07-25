#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "mcts_node.h"
#include "util.h"


typedef struct MCTSNode {
    MCTSNode* parent;
    MCTSNode** children;
    int8_t amountOfChildren;
    Player player;
    Square square;
    float wins;
    float sims;
    float UCTValue;
    Square* untriedMoves;
    int8_t amountOfUntriedMoves;
} MCTSNode;


MCTSNode* createMCTSRootNode() {
    MCTSNode* root = safe_calloc(sizeof(MCTSNode));
    root->amountOfChildren = -1;
    root->player = PLAYER2;
    root->square.board = 9;
    root->square.position = 9;
    root->amountOfUntriedMoves = -1;
    return root;
}


MCTSNode* createMCTSNode(MCTSNode* parent, Square square) {
    MCTSNode* node = safe_calloc(sizeof(MCTSNode));
    node->parent = parent;
    node->amountOfChildren = -1;
    node->player = otherPlayer(parent->player);
    node->square = square;
    node->amountOfUntriedMoves = -1;
    return node;
}


void freeNode(MCTSNode* node) {
    free(node->children);
    free(node->untriedMoves);
    safe_free(node);
}


void freeMCTSTree(MCTSNode* node) {
    for (int i = 0; i < node->amountOfChildren; i++) {
        freeMCTSTree(node->children[i]);
    }
    freeNode(node);
}


bool handleSpecialCases(MCTSNode* node, Board* board) {
    if (nextBoardIsEmpty(board) && currentPlayerIsMe(board) && getPly(board) <= 20) {
        node->amountOfUntriedMoves = 1;
        node->untriedMoves = safe_malloc(sizeof(Square));
        uint8_t currentBoard = getCurrentBoard(board);
        Square sameBoard = {currentBoard, currentBoard};
        node->untriedMoves[0] = sameBoard;
        node->children = safe_malloc(sizeof(MCTSNode*));
        return true;
    }
    if (getPly(board) == 0) {
        node->amountOfUntriedMoves = 1;
        node->untriedMoves = safe_malloc(sizeof(Square));
        Square bestFirstMove = {4, 4};
        node->untriedMoves[0] = bestFirstMove;
        node->children = safe_malloc(sizeof(MCTSNode*));
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
            node->untriedMoves = safe_malloc(amountOfMoves * sizeof(Square));
            for (int i = 0; i < amountOfMoves; i++) {
                node->untriedMoves[i] = moves[i];
            }
            node->children = safe_malloc(amountOfMoves * sizeof(MCTSNode*));
        }
    }
}


bool isLeafNode(MCTSNode* node, Board* board) {
    discoverChildNodes(node, board);
    return node->amountOfUntriedMoves > 0;
}


#define EXPLORATION_PARAMETER 0.459375f
float getUCTValue(MCTSNode* node, float parentLogSims) {
    if (node->UCTValue) {
        return node->UCTValue;
    }
    float w = node->wins;
    float n = node->sims;
    float c = EXPLORATION_PARAMETER;
    return w/n + c*sqrtf(parentLogSims / n);
}


MCTSNode* expandNode(MCTSNode* node, int childIndex) {
    MCTSNode* newChild = createMCTSNode(node, node->untriedMoves[childIndex]);
    node->amountOfUntriedMoves--;
    node->children[node->amountOfChildren++] = newChild;
    return newChild;
}


MCTSNode* selectNextChild(MCTSNode* node, Board* board) {
    discoverChildNodes(node, board);
    assert(isLeafNode(node, board) || node->amountOfChildren > 0 && "selectNextChild: node is terminal");
    if (node->amountOfUntriedMoves) {
        return expandNode(node, node->amountOfUntriedMoves - 1);
    }
    float logSims = logf(node->sims);
    MCTSNode* highestUCTChild = node->children[0];
    float highestUCT = getUCTValue(highestUCTChild, logSims);
    for (int i = 1; i < node->amountOfChildren; i++) {
        MCTSNode* child = node->children[i];
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
    assert(root->amountOfChildren > 0 || isLeafNode(root, board) && "updateRoot: root is terminal");
    MCTSNode* newRoot = NULL;
    for (int i = 0; i < root->amountOfChildren; i++) {
        MCTSNode* child = root->children[i];
        if (squaresAreEqual(square, child->square)) {
            assert(newRoot == NULL && "updateRoot: multiple children with same square found");
            child->parent = NULL;
            newRoot = child;
        } else {
            freeMCTSTree(child);
        }
    }
    if (newRoot == NULL) {
        for (int i = 0; i < root->amountOfUntriedMoves; i++) {
            if (squaresAreEqual(square, root->untriedMoves[i])) {
                newRoot = expandNode(root, i);
                newRoot->parent = NULL;
                break;
            }
        }
    }
    freeNode(root);
    assert(newRoot != NULL && "updateRoot: newRoot shouldn't be NULL");
    return newRoot;
}


void backpropagate(MCTSNode* node, Winner winner) {
    assert(winner != NONE && "backpropagate: Can't backpropagate a NONE Winner");
    node->sims++;
    if (playerIsWinner(node->player, winner)) {
        node->wins++;
    } else if (winner == DRAW) {
        node->wins += 0.5f;
    }
    if (node->parent != NULL) {
        backpropagate(node->parent, winner);
    }
}


void visitNode(MCTSNode* node, Board* board) {
    makeTemporaryMove(board, node->square);
}


#define UCT_WIN 100000
#define UCT_LOSS (-UCT_WIN)
void setNodeWinner(MCTSNode* node, Winner winner) {
    assert(winner != NONE && "setNodeWinner: Can't set NONE as winner");
    if (winner != DRAW) {
        bool win = playerIsWinner(node->player, winner);
        node->UCTValue = win? UCT_WIN : UCT_LOSS;
        if (!win) {
            node->parent->UCTValue = UCT_WIN;
        }
    }
}


Square getMostPromisingMove(MCTSNode* node, Board* board) {
    discoverChildNodes(node, board);
    assert(node->amountOfChildren > 0 && "getMostPromisingMove: node has no children");
    MCTSNode* highestSimsChild = node->children[0];
    float highestSims = highestSimsChild->sims;
    for (int i = 1; i < node->amountOfChildren; i++) {
        MCTSNode* child = node->children[i];
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
