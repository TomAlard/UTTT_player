#include <math.h>
#include <stdlib.h>
#include "mcts_node.h"
#include "util.h"


typedef struct MCTSNode {
    MCTSNode* parent;
    MCTSNode** children;
    int amountOfChildren;
    Player player;
    Square square;
    double wins;
    int sims;
    double UCTValue;
} MCTSNode;


MCTSNode* createMCTSRootNode() {
    MCTSNode* root = safe_calloc(sizeof(MCTSNode));
    root->amountOfChildren = -1;
    root->square.board = 9;
    root->square.position = 9;
    root->UCTValue = -1;
    return root;
}


MCTSNode* createMCTSNode(MCTSNode* parent, Square square) {
    MCTSNode* node = safe_calloc(sizeof(MCTSNode));
    node->parent = parent;
    node->amountOfChildren = -1;
    node->player = otherPlayer(parent->player);
    node->square.board = square.board;
    node->square.position = square.position;
    node->UCTValue = -1;
    return node;
}


void freeNode(MCTSNode* node) {
    free(node->children);
    safe_free(node);
}


void freeMCTSTree(MCTSNode* node) {
    for (int i = 0; i < node->amountOfChildren; i++) {
        freeMCTSTree(node->children[i]);
    }
    freeNode(node);
}


void discoverChildNodes(MCTSNode* node, Board* board) {
    if (node->amountOfChildren == -1) {
        Square moves[TOTAL_SMALL_SQUARES];
        node->amountOfChildren = generateMoves(board, moves);
        node->children = safe_malloc(node->amountOfChildren * sizeof(MCTSNode *));
        for (int i = 0; i < node->amountOfChildren; i++) {
            node->children[i] = createMCTSNode(node, moves[i]);
        }
    }
}


bool hasChildren(MCTSNode* node, Board* board) {
    discoverChildNodes(node, board);
    return node->amountOfChildren > 0;
}


bool isLeafNode(MCTSNode* node) {
    return node->sims == 0;
}


#define EXPLORATION_PARAMETER 1.41
double getUCTValue(MCTSNode* node) {
    if (node->UCTValue != -1) {
        return node->UCTValue;
    }
    double w = node->wins;
    double n = node->sims != 0? node->sims : 0.0001;
    double c = EXPLORATION_PARAMETER;
    double N = node->parent->sims != 0? node->parent->sims : 0.0001;
    return w/n + c*(log(N) / n);
}


MCTSNode* selectNextChild(MCTSNode* node, Board* board) {
    discoverChildNodes(node, board);
    assertMsg(node->amountOfChildren > 0, "selectNextChild: node has no children");
    MCTSNode* highestUCTChild = NULL;
    double highestUCT = -100000000000;
    for (int i = 0; i < node->amountOfChildren; i++) {
        MCTSNode* child = node->children[i];
        double UCT = getUCTValue(child);
        if (UCT > highestUCT) {
            highestUCTChild = child;
            highestUCT = UCT;
        }
    }
    assertMsg(highestUCTChild != NULL, "selectNextChild: Panic! This should be impossible.");
    return highestUCTChild;
}


MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square) {
    discoverChildNodes(root, board);
    assertMsg(root->amountOfChildren > 0, "updateRoot: root has no children");
    MCTSNode* newRoot = NULL;
    for (int i = 0; i < root->amountOfChildren; i++) {
        MCTSNode* child = root->children[i];
        if (squaresAreEqual(square, child->square)) {
            assertMsg(newRoot == NULL, "updateRoot: multiple children with same square found");
            child->parent = NULL;
            newRoot = child;
        } else {
            freeMCTSTree(child);
        }
    }
    freeNode(root);
    return newRoot;
}


void backpropagate(MCTSNode* node, Winner winner) {
    assertMsg(winner != NONE, "backpropagate: Can't backpropagate a NONE Winner");
    node->sims++;
    if (playerIsWinner(node->player, winner)) {
        node->wins++;
    } else if (winner == DRAW) {
        node->wins += 0.5;
    }
    if (node->parent != NULL) {
        backpropagate(node->parent, winner);
    }
}


void visitNode(MCTSNode* node, Board* board) {
    makeMove(board, node->square);
}


#define UCT_WIN 100000
#define UCT_LOSS (-UCT_WIN)
void setNodeWinner(MCTSNode* node, Winner winner) {
    assertMsg(winner != NONE, "setNodeWinner: Can't set NONE as winner");
    if (winner != DRAW) {
        bool win = playerIsWinner(node->player, winner);
        node->UCTValue = win? UCT_WIN : UCT_LOSS;
        if (!win) {
            node->parent->UCTValue = UCT_WIN;
        }
    }
}


Square getMostSimulatedChildSquare(MCTSNode* node) {
    assertMsg(node->amountOfChildren != -1, "getMostSimulatedChildSquare: node has no children");
    MCTSNode* highestSimsChild = NULL;
    int highestSims = -1;
    for (int i = 0; i < node->amountOfChildren; i++) {
        MCTSNode* child = node->children[i];
        int sims = child->sims;
        if (sims > highestSims) {
            highestSimsChild = child;
            highestSims = sims;
        }
    }
    return highestSimsChild->square;
}


int getSims(MCTSNode* node) {
    return node->sims;
}


double getWinrate(MCTSNode* node) {
    return node->wins / node->sims;
}
