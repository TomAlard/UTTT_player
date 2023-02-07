#include <assert.h>
#include <string.h>
#include <immintrin.h>
#include <stdalign.h>
#include "mcts_node.h"
#include "../misc/util.h"
#include "../nn/forward.h"


MCTSNode* createMCTSRootNode(Board* board) {
    MCTSNode* root = allocateNodes(board, 1);
    root->parent = NULL;
    root->children = NULL;
    root->eval = 0.0f;
    root->sims = 0.0f;
    root->simsInverted = 0.0f;
    root->square.board = 9;
    root->square.position = 9;
    root->amountOfChildren = -1;
    return root;
}


void initializeMCTSNode(MCTSNode* parent, Square square, float eval, MCTSNode* node) {
    node->parent = parent;
    node->children = NULL;
    node->eval = eval;
    node->sims = 0.0f;
    node->simsInverted = 0.0f;
    node->square = square;
    node->amountOfChildren = -1;
}


MCTSNode* copyMCTSNode(Board* board, MCTSNode* original) {
    MCTSNode* copy = allocateNodes(board, 1);
    copy->parent = original->parent;
    copy->children = original->children;
    copy->eval = original->eval;
    copy->sims = original->sims;
    copy->simsInverted = original->simsInverted;
    copy->square = original->square;
    copy->amountOfChildren = original->amountOfChildren;
    for (int i = 0; i < copy->amountOfChildren; i++) {
        copy->children[i].parent = copy;
    }
    return copy;
}


float getEvalOfMove(Board* board, Square square) {
    State temp = board->stateCheckpoint;
    updateCheckpoint(board);
    makeTemporaryMove(board, square);
    float eval = neuralNetworkEval(board);
    revertToCheckpoint(board);
    board->stateCheckpoint = temp;
    return eval;
}


void singleChild(MCTSNode* node, Board* board, Square square) {
    node->amountOfChildren = 1;
    node->children = allocateNodes(board, 1);
    float eval = getEvalOfMove(board, square);
    initializeMCTSNode(node, square, eval, &node->children[0]);
}


bool handleSpecialCases(MCTSNode* node, Board* board) {
    if (nextBoardIsEmpty(board) && getPly(board) <= 20) {
        uint8_t currentBoard = getCurrentBoard(board);
        Square sameBoard = {currentBoard, currentBoard};
        singleChild(node, board, sameBoard);
        return true;
    }
    if (currentPlayerIsMe(board) && getPly(board) == 0) {
        Square bestFirstMove = {4, 4};
        singleChild(node, board, bestFirstMove);
        return true;
    }
    return false;
}


bool isBadMove(Board* board, Square square) {
    bool sendsToDecidedBoard = (board->state.player1.bigBoard | board->state.player2.bigBoard) & (1 << square.position);
    return sendsToDecidedBoard && getPly(board) <= 30;
}


void initializeChildNodes(MCTSNode* parent, Board* board, Square* moves) {
    alignas(32) int16_t NNInputs[256];
    board->state.currentPlayer ^= 1;
    setHidden(board, NNInputs);
    board->state.currentPlayer ^= 1;
    int8_t amountOfMoves = parent->amountOfChildren;
    PlayerBitBoard* p1 = &board->state.player1;
    PlayerBitBoard* currentPlayerBitBoard = p1 + board->state.currentPlayer;
    PlayerBitBoard* otherPlayerBitBoard = p1 + !board->state.currentPlayer;
    int childIndex = 0;
    __m256i regs[16];
    for (int i = 0; i < amountOfMoves; i++) {
        Square move = moves[i];
        if (isBadMove(board, move) && parent->amountOfChildren > 1) {
            parent->amountOfChildren--;
            continue;
        }
        for (int j = 0; j < 16; j++) {
            regs[j] = _mm256_load_si256((__m256i*) &NNInputs[j * 16]);
        }
        MCTSNode* child = &parent->children[childIndex++];
        addFeature(move.position + 99 + 9*move.board, regs);
        uint16_t smallBoard = currentPlayerBitBoard->smallBoards[move.board];
        BIT_SET(smallBoard, move.position);
        bool smallBoardIsDecided;
        if (isWin(smallBoard)) {
            smallBoardIsDecided = BIT_CHECK(board->state.player1.bigBoard | board->state.player2.bigBoard
                                            | (1 << move.board), move.position);
            addFeature(move.board + 90, regs);
        } else if (isDraw(smallBoard, otherPlayerBitBoard->smallBoards[move.board])) {
            smallBoardIsDecided = BIT_CHECK(board->state.player1.bigBoard | board->state.player2.bigBoard
                                            | (1 << move.board), move.position);
            addFeature(move.board, regs);
            addFeature(move.board + 90, regs);
        } else {
            smallBoardIsDecided = BIT_CHECK(board->state.player1.bigBoard | board->state.player2.bigBoard, move.position);
        }
        addFeature((smallBoardIsDecided? ANY_BOARD : move.position) + 180, regs);
        alignas(32) int16_t result[256];
        for (int j = 0; j < 16; j++) {
            _mm256_store_si256((__m256i*) &result[j * 16], regs[j]);
        }
        float eval = neuralNetworkEvalFromHidden(result);
        initializeMCTSNode(parent, move, eval, child);
    }
}


void discoverChildNodes(MCTSNode* node, Board* board) {
    if (node->amountOfChildren == -1 && !handleSpecialCases(node, board)) {
        Square movesArray[TOTAL_SMALL_SQUARES];
        int8_t amountOfMoves;
        Square* moves = generateMoves(board, movesArray, &amountOfMoves);
        if (getPly(board) > 30) {
            Player player = getCurrentPlayer(board);
            for (int i = 0; i < amountOfMoves; i++) {
                if (getWinnerAfterMove(board, moves[i]) == player + 1) {
                    singleChild(node, board, moves[i]);
                    return;
                }
            }
        }
        node->amountOfChildren = amountOfMoves;
        node->children = allocateNodes(board, amountOfMoves);
        initializeChildNodes(node, board, moves);
    }
}


bool isLeafNode(MCTSNode* node) {
    return node->sims == 0;
}


float fastSquareRoot(float x) {
    __m128 in = _mm_set_ss(x);
    return _mm_cvtss_f32(_mm_mul_ss(in, _mm_rsqrt_ss(in)));
}


#define EXPLORATION_PARAMETER 0.41f
#define FIRST_PLAY_URGENCY 0.40f
float getUCTValue(MCTSNode* node, float parentLogSims) {
    float exploitation = node->eval;
    float exploration = node->sims == 0? FIRST_PLAY_URGENCY : fastSquareRoot(parentLogSims * node->simsInverted);
    return exploitation + exploration;
}


// From: https://github.com/etheory/fastapprox/blob/master/fastapprox/src/fastlog.h
float fastLog2(float x) {
    union { float f; uint32_t i; } vx = { x };
    float y = (float)vx.i;
    y *= 1.1920928955078125e-7f;
    return y - 126.94269504f;
}


MCTSNode* selectNextChild(MCTSNode* node) {
    assert(node->amountOfChildren > 0);
    float logSims = EXPLORATION_PARAMETER*EXPLORATION_PARAMETER * fastLog2(node->sims);
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
    assert(highestUCTChild != NULL);
    return highestUCTChild;
}


MCTSNode* expandLeaf(MCTSNode* leaf, Board* board) {
    assert(isLeafNode(leaf));
    discoverChildNodes(leaf, board);
    return selectNextChild(leaf);
}


MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square) {
    MCTSNode* newRoot = NULL;
    for (int i = 0; i < root->amountOfChildren; i++) {
        MCTSNode* child = &root->children[i];
        if (squaresAreEqual(square, child->square)) {
            newRoot = child;
            break;
        }
    }
    if (newRoot == NULL) {
        Square movesArray[TOTAL_SMALL_SQUARES];
        int8_t amountOfMoves;
        Square* moves = generateMoves(board, movesArray, &amountOfMoves);
        for (int i = 0; i < amountOfMoves; i++) {
            if (squaresAreEqual(moves[i], square)) {
                newRoot = allocateNodes(board, 1);
                initializeMCTSNode(NULL, square, 0.0f, newRoot);
                break;
            }
        }
        assert(newRoot != NULL);
    } else {
        newRoot->parent = NULL;
        newRoot = copyMCTSNode(board, newRoot);
    }
    return newRoot;
}


void backpropagate(MCTSNode* node, Winner winner, Player player) {
    assert(winner != NONE && "backpropagate: Can't backpropagate a NONE Winner");
    node->eval = winner == DRAW ? 0.5f : player + 1 == winner ? 1.0f : 0.0f;
    backpropagateEval(node);
}


void backpropagateEval(MCTSNode* node) {
    assert(node != NULL);
    MCTSNode* currentNode = node;
    if (currentNode->amountOfChildren <= 0) {
        currentNode = currentNode->parent;
    }
    while (currentNode != NULL) {
        float maxChildEval = 0.0f;
        for (int i = 0; i < currentNode->amountOfChildren; i++) {
            float eval = currentNode->children[i].eval;
            maxChildEval = maxChildEval >= eval? maxChildEval : eval;
        }
        currentNode->eval = 1 - maxChildEval;
        currentNode->sims++;
        currentNode->simsInverted = 1.0f / currentNode->sims;
        currentNode = currentNode->parent;
    }
}


void visitNode(MCTSNode* node, Board* board) {
    makeTemporaryMove(board, node->square);
}


Square getMostPromisingMove(MCTSNode* node) {
    assert(node->amountOfChildren > 0 && "getMostPromisingMove: node has no children");
    MCTSNode* highestScoreChild = &node->children[0];
    float highestScore = getEval(highestScoreChild) + fastLog2(highestScoreChild->sims);
    for (int i = 1; i < node->amountOfChildren; i++) {
        MCTSNode* child = &node->children[i];
        float score = getEval(child) + fastLog2(child->sims);
        if (score > highestScore) {
            highestScoreChild = child;
            highestScore = score;
        }
    }
    return highestScoreChild->square;
}


float getEval(MCTSNode* node) {
    return node->eval;
}
