#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <emmintrin.h>
#include "mcts_node.h"
#include "../misc/util.h"
#include "priors.h"


MCTSNode* createMCTSRootNode() {
    MCTSNode* root = safe_calloc(sizeof(MCTSNode));
    root->amountOfChildren = -1;
    root->square.board = 9;
    root->square.position = 9;
    root->amountOfUntriedMoves = -1;
    initializePriorsLookupTable();
    return root;
}


void initializeMCTSNode(MCTSNode* parent, Square square, MCTSNode* node) {
    node->parent = parent;
    node->children = NULL;
    node->wins = 0.0f;
    node->sims = 0.0f;
    node->simsInverted = 0.0f;
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
    copy->simsInverted = original->simsInverted;
    copy->square = original->square;
    copy->amountOfChildren = original->amountOfChildren;
    copy->amountOfUntriedMoves = original->amountOfUntriedMoves;
    for (int i = 0; i < copy->amountOfChildren + copy->amountOfUntriedMoves; i++) {
        copy->children[i].parent = copy;
    }
    return copy;
}


void freeMCTSTreeRecursive(MCTSNode* node) {
    for (int i = 0; i < node->amountOfChildren; i++) {
        freeMCTSTreeRecursive(&node->children[i]);
    }
    free(node->children);
}


void freeMCTSTree(MCTSNode* root) {
    freeMCTSTreeRecursive(root);
    if (root->parent == NULL) {
        free(root);
    }
}


void singleChild(MCTSNode* node, Square square) {
    node->amountOfUntriedMoves = 1;
    node->children = safe_malloc(sizeof(MCTSNode));
    initializeMCTSNode(node, square, &node->children[0]);
}


bool handleSpecialCases(MCTSNode* node, Board* board) {
    if (nextBoardIsEmpty(board) && getPly(board) <= 20) {
        uint8_t currentBoard = getCurrentBoard(board);
        Square sameBoard = {currentBoard, currentBoard};
        singleChild(node, sameBoard);
        return true;
    }
    if (currentPlayerIsMe(board) && getPly(board) == 0) {
        Square bestFirstMove = {4, 4};
        singleChild(node, bestFirstMove);
        return true;
    }
    return false;
}


void discoverChildNodes(MCTSNode* node, Board* board, RNG* rng) {
    if (node->amountOfChildren == -1) {
        node->amountOfChildren = 0;
        if (!handleSpecialCases(node, board)) {
            Square movesArray[TOTAL_SMALL_SQUARES];
            int8_t amountOfMoves;
            Square* moves = generateMoves(board, movesArray, &amountOfMoves);
            if (getPly(board) > 30) {
                Player player = getCurrentPlayer(board);
                for (int i = 0; i < amountOfMoves; i++) {
                    if (getWinnerAfterMove(board, moves[i]) == player + 1) {
                        singleChild(node, moves[i]);
                        return;
                    }
                }
            }
            int range[amountOfMoves];
            for (int i = 0; i < amountOfMoves; i++) {
                range[i] = i;
            }
            shuffle(range, amountOfMoves, rng);
            node->amountOfUntriedMoves = amountOfMoves;
            node->children = safe_malloc(amountOfMoves * sizeof(MCTSNode));
            for (int i = 0; i < amountOfMoves; i++) {
                Square move = moves[range[i]];
                MCTSNode* child = &node->children[i];
                initializeMCTSNode(node, move, child);
            }
            applyPriors(board, node);
        }
    }
}


bool isLeafNode(MCTSNode* node, Board* board, RNG* rng) {
    if (node->sims > 0) {
        discoverChildNodes(node, board, rng);
    }
    return node->sims == 0;
}


void fastSquareRoot(float* restrict pOut, float* restrict pIn) {
    __m128 in = _mm_load_ss(pIn);
    _mm_store_ss(pOut, _mm_mul_ss(in, _mm_rsqrt_ss(in)));
}


#define EXPLORATION_PARAMETER 0.41f
float getUCTValue(MCTSNode* node, float parentLogSims) {
    float c = EXPLORATION_PARAMETER;
    float sqrtIn = parentLogSims * node->simsInverted;
    float sqrtOut;
    fastSquareRoot(&sqrtOut, &sqrtIn);
    return node->wins*node->simsInverted + c*sqrtOut;
}


// From: https://github.com/etheory/fastapprox/blob/master/fastapprox/src/fastlog.h
float fastLog2(float x) {
    union { float f; uint32_t i; } vx = { x };
    float y = (float)vx.i;
    y *= 1.1920928955078125e-7f;
    return y - 126.94269504f;
}


#define FIRST_PLAY_URGENCY 1.10f
MCTSNode* selectNextChild(MCTSNode* node) {
    assert(node->amountOfChildren > 0 || node->amountOfUntriedMoves > 0);
    float logSims = fastLog2(node->sims);
    MCTSNode* highestUCTChild = NULL;
    float highestUCT = -1.0f;
    for (int i = 0; i < node->amountOfChildren; i++) {
        MCTSNode* child = &node->children[i];
        float UCT = getUCTValue(child, logSims);
        if (UCT > highestUCT) {
            highestUCTChild = child;
            highestUCT = UCT;
        }
    }
    if (node->amountOfUntriedMoves > 0 && highestUCT < FIRST_PLAY_URGENCY) {
        highestUCTChild = expandNextChild(node);
    }
    assert(highestUCTChild != NULL);
    return highestUCTChild;
}


MCTSNode* expandNextChild(MCTSNode* node) {
    assert(node->amountOfUntriedMoves > 0);
    node->amountOfUntriedMoves--;
    return &node->children[node->amountOfChildren++];
}


MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square) {
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
        Square movesArray[TOTAL_SMALL_SQUARES];
        int8_t amountOfMoves;
        Square* moves = generateMoves(board, movesArray, &amountOfMoves);
        for (int i = 0; i < amountOfMoves; i++) {
            if (squaresAreEqual(moves[i], square)) {
                MCTSNode temp;
                initializeMCTSNode(NULL, square, &temp);
                newRoot = copyMCTSNode(&temp);
                break;
            }
        }
        assert(newRoot != NULL);
    } else {
        newRoot->parent = NULL;
        newRoot = copyMCTSNode(newRoot);
    }
    free(root->children);
    free(root);
    return newRoot;
}


void backpropagate(MCTSNode* node, Winner winner, Player player) {
    assert(winner != NONE && "backpropagate: Can't backpropagate a NONE Winner");
    MCTSNode* currentNode = node;
    float reward = winner == DRAW? 0.5f : player + 1 == winner? 1.0f : 0.0f;
    while (currentNode != NULL) {
        currentNode->wins += reward;
        currentNode->simsInverted = 1.0f / ++currentNode->sims;
        reward = 1 - reward;
        currentNode = currentNode->parent;
    }
}


void visitNode(MCTSNode* node, Board* board) {
    makeTemporaryMove(board, node->square);
}


Square getMostPromisingMove(MCTSNode* node) {
    assert(node->amountOfChildren > 0 && "getMostPromisingMove: node has no children");
    MCTSNode* highestWinrateChild = &node->children[0];
    float highestWinrate = getWinrate(highestWinrateChild);
    for (int i = 1; i < node->amountOfChildren; i++) {
        MCTSNode* child = &node->children[i];
        float winrate = getWinrate(child);
        if (winrate > highestWinrate) {
            highestWinrateChild = child;
            highestWinrate = winrate;
        }
    }
    return highestWinrateChild->square;
}


float getWinrate(MCTSNode* node) {
    return node->wins * node->simsInverted;
}
