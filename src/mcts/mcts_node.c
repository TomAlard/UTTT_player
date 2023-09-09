#include <assert.h>
#include <string.h>
#include <immintrin.h>
#include <stdalign.h>
#include "mcts_node.h"
#include "../misc/util.h"
#include "../nn/forward.h"


int createMCTSRootNode(Board* board) {
    int rootIndex = allocateNodes(board, 1);
    MCTSNode* root = &board->nodes[rootIndex];
    root->childrenIndex = -1;
    root->eval = 0.0f;
    root->evalSum = 0.0f;
    root->sims = 0.0f;
    root->square.board = 9;
    root->square.position = 9;
    root->amountOfChildren = -1;
    return rootIndex;
}


void initializeMCTSNode(Square square, float eval, MCTSNode* node) {
    node->childrenIndex = -1;
    node->eval = eval;
    node->evalSum = eval;
    node->sims = 0.0f;
    node->square = square;
    node->amountOfChildren = -1;
}


float getEvalOfMove(Board* board, Square square) {
    State temp = board->stateCheckpoint;
    updateCheckpoint(board);
    makeTemporaryMove(board, square);
    float eval;
    Player player = OTHER_PLAYER(board->state.currentPlayer);
    Winner winner = board->state.winner;
    if (winner != NONE) {
        eval = winner == DRAW? 0.5f : player + 1 == winner? 10000.0f : -10000.0f;
    } else {
        eval = neuralNetworkEval(board);
    }
    revertToCheckpoint(board);
    board->stateCheckpoint = temp;
    return eval;
}


void singleChild(int nodeIndex, Board* board, Square square) {
    MCTSNode* node = &board->nodes[nodeIndex];
    node->amountOfChildren = 1;
    node->childrenIndex = allocateNodes(board, 1);
    float eval = getEvalOfMove(board, square);
    initializeMCTSNode(square, eval, &board->nodes[node->childrenIndex + 0]);
}


bool handleSpecialCases(int nodeIndex, Board* board) {
    if (board->state.ply <= 20 && nextBoardIsEmpty(board)) {
        uint8_t currentBoard = board->state.currentBoard;
        Square sameBoard = {currentBoard, currentBoard};
        singleChild(nodeIndex, board, sameBoard);
        return true;
    }
    if (board->state.ply == 0 && board->state.currentPlayer == board->me) {
        Square bestFirstMove = {4, 4};
        singleChild(nodeIndex, board, bestFirstMove);
        return true;
    }
    return false;
}


bool isBadMove(Board* board, Square square, Winner winner, Player player) {
    bool isProvenLoss = (winner == WIN_P1 && player == PLAYER2) || (winner == WIN_P2 && player == PLAYER1);
    bool sendsToDecidedBoard = (board->state.player1.bigBoard | board->state.player2.bigBoard) & (1 << square.position);
    return isProvenLoss || (sendsToDecidedBoard && board->state.ply <= 30);
}


void initializeChildNodes(int parentIndex, Board* board, Square* moves, Winner* winners) {
    MCTSNode* parent = &board->nodes[parentIndex];
    __m256i regs[16];
    board->state.currentPlayer ^= 1;
    boardToInput(board, regs);
    board->state.currentPlayer ^= 1;
    alignas(32) int16_t NNInputs[256];
    for (int i = 0; i < 16; i++) {
        _mm256_store_si256((__m256i*) &NNInputs[i * 16], regs[i]);
    }
    int8_t amountOfMoves = parent->amountOfChildren;
    PlayerBitBoard* p1 = &board->state.player1;
    PlayerBitBoard* currentPlayerBitBoard = p1 + board->state.currentPlayer;
    PlayerBitBoard* otherPlayerBitBoard = p1 + !board->state.currentPlayer;
    int childIndex = 0;
    for (int i = 0; i < amountOfMoves; i++) {
        Square move = moves[i];
        if (isBadMove(board, move, winners[i], board->state.currentPlayer) && parent->amountOfChildren > 1) {
            parent->amountOfChildren--;
            continue;
        } else if (winners[i] == DRAW) {
            MCTSNode* child = &board->nodes[parent->childrenIndex + childIndex++];
            initializeMCTSNode(move, 0.5f, child);
            continue;
        }
        MCTSNode* child = &board->nodes[parent->childrenIndex + childIndex++];
        uint16_t smallBoard = extractSmallBoard(currentPlayerBitBoard, move.board);
        BIT_SET(smallBoard, move.position);
        bool smallBoardIsDecided;
        for (int j = 0; j < 16; j++) {
            regs[j] = _mm256_load_si256((__m256i*) &NNInputs[j * 16]);
        }
        if (isWin(smallBoard)) {
            smallBoardIsDecided = BIT_CHECK(board->state.player1.bigBoard | board->state.player2.bigBoard
                                            | (1 << move.board), move.position);
            addFeature(move.board + 90, regs);
        } else if (isDraw(smallBoard, extractSmallBoard(otherPlayerBitBoard, move.board))) {
            smallBoardIsDecided = BIT_CHECK(board->state.player1.bigBoard | board->state.player2.bigBoard
                                            | (1 << move.board), move.position);
            addFeature(move.board, regs);
            addFeature(move.board + 90, regs);
        } else {
            smallBoardIsDecided = BIT_CHECK(board->state.player1.bigBoard | board->state.player2.bigBoard, move.position);
        }
        addFeature(move.position + 99 + 9*move.board, regs);
        addFeature((smallBoardIsDecided? ANY_BOARD : move.position) + 180, regs);
        float eval = neuralNetworkEvalFromHidden(regs);
        initializeMCTSNode(move, eval, child);
    }
}


void discoverChildNodes(int nodeIndex, Board* board) {
    MCTSNode* node = &board->nodes[nodeIndex];
    if (node->amountOfChildren == -1 && !handleSpecialCases(nodeIndex, board)) {
        Square moves[TOTAL_SMALL_SQUARES];
        int8_t amountOfMoves = generateMoves(board, moves);
        Player player = board->state.currentPlayer;
        Winner winners[amountOfMoves];
        memset(winners, NONE, amountOfMoves * sizeof(Winner));
        if (board->state.ply > 30) {
            for (int i = 0; i < amountOfMoves; i++) {
                Winner winner = getWinnerAfterMove(board, moves[i]);
                if (winner == player + 1) {
                    node->amountOfChildren = 1;
                    node->childrenIndex = allocateNodes(board, 1);
                    initializeMCTSNode(moves[i], 10000.0f, &board->nodes[node->childrenIndex]);
                    return;
                } else {
                    winners[i] = winner;
                }
            }
        }
        node->amountOfChildren = amountOfMoves;
        node->childrenIndex = allocateNodes(board, amountOfMoves);
        initializeChildNodes(nodeIndex, board, moves, winners);
    }
}


bool isLeafNode(int nodeIndex, Board* board) {
    return board->nodes[nodeIndex].sims == 0;
}


float fastSquareRoot(float x) {
    __m128 in = _mm_set_ss(x);
    return _mm_cvtss_f32(_mm_mul_ss(in, _mm_rsqrt_ss(in)));
}


#define EXPLORATION_PARAMETER 0.41f
#define FIRST_PLAY_URGENCY 0.40f
#define EXPLOITATION_LAMBDA 0.60f
float getUCTValue(MCTSNode* node, float parentLogSims) {
    float exploitation = (EXPLOITATION_LAMBDA * node->eval) + ((1 - EXPLOITATION_LAMBDA) * (node->evalSum / (node->sims + 1)));
    float exploration = node->sims == 0? FIRST_PLAY_URGENCY : fastSquareRoot(parentLogSims / node->sims);
    return exploitation + exploration;
}


// From: https://github.com/etheory/fastapprox/blob/master/fastapprox/src/fastlog.h
float fastLog2(float x) {
    union { float f; uint32_t i; } vx = { x };
    float y = (float)vx.i;
    y *= 1.1920928955078125e-7f;
    return y - 126.94269504f;
}


int selectNextChild(Board* board, int nodeIndex) {
    MCTSNode* node = &board->nodes[nodeIndex];
    assert(node->amountOfChildren > 0);
    float logSims = EXPLORATION_PARAMETER*EXPLORATION_PARAMETER * fastLog2(node->sims);
    int highestUCTChildIndex = -1;
    float highestUCT = -100000.0f;
    for (int i = 0; i < node->amountOfChildren; i++) {
        int childIndex = node->childrenIndex + i;
        float UCT = getUCTValue(&board->nodes[childIndex], logSims);
        if (UCT > highestUCT) {
            highestUCTChildIndex = childIndex;
            highestUCT = UCT;
        }
    }
    assert(highestUCTChildIndex != -1);
    return highestUCTChildIndex;
}


MCTSNode* expandLeaf(int leafIndex, Board* board) {
    assert(isLeafNode(leafIndex, board));
    discoverChildNodes(leafIndex, board);
    return &board->nodes[leafIndex];
}


int updateRoot(MCTSNode* root, Board* board, Square square) {
    for (int i = 0; i < root->amountOfChildren; i++) {
        MCTSNode* child = &board->nodes[root->childrenIndex + i];
        if (squaresAreEqual(square, child->square)) {
            return root->childrenIndex + i;
        }
    }

    Square moves[TOTAL_SMALL_SQUARES];
    int8_t amountOfMoves = generateMoves(board, moves);
    for (int i = 0; i < amountOfMoves; i++) {
        if (squaresAreEqual(moves[i], square)) {
            int newRootIndex = allocateNodes(board, 1);
            initializeMCTSNode(square, 0.0f, &board->nodes[newRootIndex]);
            return newRootIndex;
        }
    }
    exit(123);
}


void backpropagate(Board* board, int nodeIndex, Winner winner, Player player, const int* parentIndices) {
    assert(winner != NONE && "backpropagate: Can't backpropagate a NONE Winner");
    MCTSNode* node = &board->nodes[nodeIndex];
    node->eval = winner == DRAW ? 0.5f : player + 1 == winner ? 10000.0f : -10000.0f;
    node->evalSum = node->eval * (node->sims + 1);
    backpropagateEval(board, node, parentIndices);
}


void backpropagateEval(Board* board, MCTSNode* node, const int* parentIndices) {
    assert(node != NULL);
    MCTSNode* currentNode = node;
    int i = 0;
    if (currentNode->amountOfChildren <= 0) {
        int nextNodeIndex = parentIndices[i++];
        currentNode = nextNodeIndex == -1 ? NULL : &board->nodes[nextNodeIndex];
    }
    while (currentNode != NULL) {
        float maxChildEval = -10000.0f;
        for (int j = 0; j < currentNode->amountOfChildren; j++) {
            MCTSNode* child = &board->nodes[currentNode->childrenIndex + j];
            float eval = child->eval;
            maxChildEval = maxChildEval >= eval? maxChildEval : eval;
        }
        currentNode->eval = 1 - maxChildEval;
        currentNode->evalSum += currentNode->eval;
        currentNode->sims++;
        int nextNodeIndex = parentIndices[i++];
        currentNode = nextNodeIndex == -1 ? NULL : &board->nodes[nextNodeIndex];
    }
}


void visitNode(int nodeIndex, Board* board) {
    makeTemporaryMove(board, board->nodes[nodeIndex].square);
}


Square getMostPromisingMove(Board* board, MCTSNode* node) {
    assert(node->amountOfChildren > 0 && "getMostPromisingMove: node has no children");
    MCTSNode* highestScoreChild = &board->nodes[node->childrenIndex + 0];
    float highestScore = highestScoreChild->eval + fastLog2(highestScoreChild->sims);
    for (int i = 1; i < node->amountOfChildren; i++) {
        MCTSNode* child = &board->nodes[node->childrenIndex + i];
        float score = child->eval + fastLog2(child->sims);
        if (score > highestScore) {
            highestScoreChild = child;
            highestScore = score;
        }
    }
    return highestScoreChild->square;
}
