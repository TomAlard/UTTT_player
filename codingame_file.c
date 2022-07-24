#pragma GCC optimize("Ofast", "unroll-loops", "omit-frame-pointer", "inline")
#pragma GCC option("arch=native", "tune=native", "no-zero-upper")
#pragma GCC target("rdrnd", "popcnt", "avx", "bmi2")


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <emmintrin.h>
#include <smmintrin.h>



// START HEADERS

typedef struct Square {
    uint8_t board;
    uint8_t position;
} Square;

Square createSquare(uint8_t board, uint8_t position);

bool squaresAreEqual(Square square1, Square square2);

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CHECK(a,b) ((a) & (1ULL<<(b)))

void crash(char* errorMessage);

Square toOurNotation(Square rowAndColumn);

Square toGameNotation(Square square);

typedef struct pcg32_random_t {
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

void pcg32_srandom_r(pcg32_random_t* rng, uint64_t init_state, uint64_t init_seq);

uint8_t pcg32_boundedrand_r(pcg32_random_t* rng, uint8_t bound);

typedef enum Player {
    PLAYER1,
    PLAYER2
} Player;

Player otherPlayer(Player player);

typedef enum Occupation {
    UNOCCUPIED,
    OCCUPIED_P1,
    OCCUPIED_P2
} Occupation;

typedef enum Winner {
    NONE,
    WIN_P1,
    WIN_P2,
    DRAW
} Winner;

bool playerIsWinner(Player player, Winner winner);

typedef struct PlayerBitBoard PlayerBitBoard;

PlayerBitBoard* createPlayerBitBoard();

void freePlayerBitBoard(PlayerBitBoard* playerBitBoard);

uint16_t getBigBoard(PlayerBitBoard* playerBitBoard);

bool boardIsWon(PlayerBitBoard* playerBitBoard, uint8_t board);

uint16_t getSmallBoard(PlayerBitBoard* playerBitBoard, uint8_t board);

bool squareIsOccupied(PlayerBitBoard* playerBitBoard, Square square);

bool isWin(uint16_t smallBoard);

bool setSquareOccupied(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayerBitBoard, Square square);

void revertToPlayerCheckpoint(PlayerBitBoard* playerBitBoard);

void updatePlayerCheckpoint(PlayerBitBoard* playerBitBoard);

#define TOTAL_SMALL_SQUARES 81

typedef struct Board Board;

Board* createBoard();

void freeBoard(Board* board);

Square* generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves);

void makeRandomTemporaryMove(Board* board, pcg32_random_t* rng);

bool nextBoardIsEmpty(Board* board);

uint8_t getCurrentBoard(Board* board);

void revertToCheckpoint(Board* board);

void makeTemporaryMove(Board* board, Square square);

void makePermanentMove(Board* board, Square square);

Winner getWinner(Board* board);

void setMe(Board* board, Player player);

bool currentPlayerIsMe(Board* board);

uint8_t getPly(Board* board);

typedef struct MCTSNode MCTSNode;

MCTSNode* createMCTSRootNode();

void freeMCTSTree(MCTSNode* root);

bool isLeafNode(MCTSNode* node, Board* board);

MCTSNode* selectNextChild(MCTSNode* node, Board* board);

MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square);

void backpropagate(MCTSNode* node, Winner winner);

void visitNode(MCTSNode* node, Board* board);

void setNodeWinner(MCTSNode* node, Winner winner);

Square getMostPromisingMove(MCTSNode* node, Board* board);

int getSims(MCTSNode* node);

float getWinrate(MCTSNode* node);

int findNextMove(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime);

typedef struct HandleTurnResult {
    Square move;
    MCTSNode* newRoot;
    int amountOfSimulations;
} HandleTurnResult;

HandleTurnResult handleTurn(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime, Square enemyMove);

// END HEADERS












// START SQUARE
Square createSquare(uint8_t board, uint8_t position) {
    Square result = {board, position};
    return result;
}


bool squaresAreEqual(Square square1, Square square2) {
    return square1.board == square2.board && square1.position == square2.position;
}
// END SQUARE














// START UTIL
void crash(char* errorMessage) {
    fprintf(stderr, "%s\n", errorMessage);
    exit(1);
}


Square toOurNotation(Square rowAndColumn) {
    uint8_t row = rowAndColumn.board;
    uint8_t column = rowAndColumn.position;
    if (row == 255 && column == 255) {
        Square result = {9, 9};
        return result;
    }
    uint8_t board = 3 * (row / 3) + (column / 3);
    uint8_t position = 3 * (row % 3) + (column % 3);
    Square result = {board, position};
    return result;
}


Square toGameNotation(Square square) {
    uint8_t row = (square.position / 3) + 3*(square.board / 3);
    uint8_t col = (square.position % 3) + 3*(square.board % 3);
    Square result = {row, col};
    return result;
}
// END UTIL












// START RANDOM
uint32_t pcg32_random_r(pcg32_random_t* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}


void pcg32_srandom_r(pcg32_random_t* rng, uint64_t init_state, uint64_t init_seq) {
    rng->state = 0U;
    rng->inc = (init_seq << 1u) | 1u;
    pcg32_random_r(rng);
    rng->state += init_state;
    pcg32_random_r(rng);
}


uint8_t pcg32_boundedrand_r(pcg32_random_t* rng, uint8_t bound) {
    return ((uint64_t)pcg32_random_r(rng) * (uint64_t)bound) >> 32;
}
// END RANDOM











// START PLAYER
Player otherPlayer(Player player) {
    return player == PLAYER1? PLAYER2 : PLAYER1;
}


bool playerIsWinner(Player player, Winner winner) {
    return (player == PLAYER1 && winner == WIN_P1) || (player == PLAYER2 && winner == WIN_P2);
}
// END PLAYER












// START PLAYER_BIT_BOARD
typedef struct PlayerBitBoard {
    uint16_t smallBoards[9];
    uint16_t bigBoard;
    uint16_t checkpointSmallBoards[9];
    uint16_t checkpointBigBoard;
} PlayerBitBoard;


PlayerBitBoard* createPlayerBitBoard() {
    return calloc(1, sizeof(PlayerBitBoard));
}


void freePlayerBitBoard(PlayerBitBoard* playerBitBoard) {
    free(playerBitBoard);
}


uint16_t getBigBoard(PlayerBitBoard* playerBitBoard) {
    return playerBitBoard->bigBoard;
}


bool boardIsWon(PlayerBitBoard* playerBitBoard, uint8_t board) {
    return BIT_CHECK(playerBitBoard->bigBoard, board);
}


uint16_t getSmallBoard(PlayerBitBoard* playerBitBoard, uint8_t board) {
    return playerBitBoard->smallBoards[board];
}


bool squareIsOccupied(PlayerBitBoard* playerBitBoard, Square square) {
    return BIT_CHECK(playerBitBoard->smallBoards[square.board], square.position);
}


bool isWin(uint16_t smallBoard) {
    const __m128i masks = _mm_setr_epi16(0x7, 0x38, 0x1c0, 0x49, 0x92, 0x124, 0x111, 0x54);
    const __m128i v1 = _mm_set1_epi16(1);
    __m128i boards128 = _mm_set1_epi16(smallBoard);
    __m128i andResult = _mm_and_si128(masks, boards128);
    __m128i result = _mm_cmpeq_epi16(andResult, masks);
    return !_mm_test_all_zeros(result, v1);
}


bool isDraw(uint16_t smallBoard, uint16_t otherPlayerSmallBoard) {
    return (smallBoard | otherPlayerSmallBoard) == 511;
}


bool setSquareOccupied(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayerBitBoard, Square square) {
    BIT_SET(playerBitBoard->smallBoards[square.board], square.position);
    uint16_t smallBoard = playerBitBoard->smallBoards[square.board];
    uint16_t otherPlayerSmallBoard = otherPlayerBitBoard->smallBoards[square.board];
    if (isWin(smallBoard)) {
        BIT_SET(playerBitBoard->bigBoard, square.board);
        return true;
    } if (isDraw(smallBoard, otherPlayerSmallBoard)) {
        BIT_SET(playerBitBoard->bigBoard, square.board);
        BIT_SET(otherPlayerBitBoard->bigBoard, square.board);
        return true;
    }
    return false;
}


void revertToPlayerCheckpoint(PlayerBitBoard* playerBitBoard) {
    for (int i = 0; i < 9; i++) {
        playerBitBoard->smallBoards[i] = playerBitBoard->checkpointSmallBoards[i];
    }
    playerBitBoard->bigBoard = playerBitBoard->checkpointBigBoard;
}


void updatePlayerCheckpoint(PlayerBitBoard* playerBitBoard) {
    for (int i = 0; i < 9; i++) {
        playerBitBoard->checkpointSmallBoards[i] = playerBitBoard->smallBoards[i];
    }
    playerBitBoard->checkpointBigBoard = playerBitBoard->bigBoard;
}
// END PLAYER_BIT_BOARD
















// START BOARD
#define ANY_BOARD 9


typedef struct AdditionalState {
    Player currentPlayer;
    uint8_t currentBoard;
    Winner winner;
    uint8_t ply;
    uint8_t totalAmountOfOpenSquares;
    uint8_t amountOfOpenSquaresBySmallBoard[9];
} AdditionalState;


typedef struct Board {
    PlayerBitBoard* player1;
    PlayerBitBoard* player2;
    AdditionalState AS;
    AdditionalState ASCheckpoint;
    Square openSquares[512][9][9];
    int8_t amountOfOpenSquares[512];
    Player me;
} Board;


int8_t setOpenSquares(Square openSquares[9], uint8_t boardIndex, uint16_t bitBoard) {
    int8_t amountOfMoves = 0;
    while (bitBoard) {
        Square square = {boardIndex, __builtin_ffs(bitBoard) - 1};
        openSquares[amountOfMoves++] = square;
        bitBoard &= bitBoard - 1;
    }
    return amountOfMoves;
}


Board* createBoard() {
    Board* board = malloc(sizeof(Board));
    board->player1 = createPlayerBitBoard();
    board->player2 = createPlayerBitBoard();
    board->AS.currentPlayer = PLAYER1;
    board->AS.currentBoard = ANY_BOARD;
    board->AS.winner = NONE;
    board->AS.ply = 1;
    board->AS.totalAmountOfOpenSquares = 81;
    for (int boardIndex = 0; boardIndex < 9; boardIndex++) {
        board->AS.amountOfOpenSquaresBySmallBoard[boardIndex] = 9;
    }
    board->ASCheckpoint = board->AS;
    for (int boardIndex = 0; boardIndex < 9; boardIndex++) {
        for (int bitBoard = 0; bitBoard < 512; bitBoard++) {
            board->amountOfOpenSquares[bitBoard] =
                    setOpenSquares(board->openSquares[bitBoard][boardIndex], boardIndex, bitBoard);
        }
    }
    board->me = PLAYER2;
    return board;
}


void freeBoard(Board* board) {
    freePlayerBitBoard(board->player1);
    freePlayerBitBoard(board->player2);
    free(board);
}


Square* getMovesSingleBoard(Board* board, uint8_t boardIndex, int8_t* amountOfMoves) {
    uint16_t bitBoard = ~(getSmallBoard(board->player1, boardIndex) | getSmallBoard(board->player2, boardIndex)) & 511;
    *amountOfMoves = board->amountOfOpenSquares[bitBoard];
    return board->openSquares[bitBoard][boardIndex];
}


int8_t copyMovesSingleBoard(Board* board, uint8_t boardIndex, Square moves[TOTAL_SMALL_SQUARES], int8_t amountOfMoves) {
    uint16_t bitBoard = ~(getSmallBoard(board->player1, boardIndex) | getSmallBoard(board->player2, boardIndex)) & 511;
    memcpy(&moves[amountOfMoves], board->openSquares[bitBoard][boardIndex],
           board->amountOfOpenSquares[bitBoard] * sizeof(Square));
    return (int8_t)(amountOfMoves + board->amountOfOpenSquares[bitBoard]);
}


int8_t generateMovesAnyBoard(Board* board, Square moves[TOTAL_SMALL_SQUARES]) {
    int8_t amountOfMoves = 0;
    uint16_t undecidedSmallBoards = ~(getBigBoard(board->player1) | getBigBoard(board->player2)) & 511;
    while (undecidedSmallBoards) {
        uint8_t boardIndex = __builtin_ffs(undecidedSmallBoards) - 1;
        amountOfMoves = copyMovesSingleBoard(board, boardIndex, moves, amountOfMoves);
        undecidedSmallBoards &= undecidedSmallBoards - 1;
    }
    return amountOfMoves;
}


Square* generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves) {
    if (board->AS.winner != NONE) {
        *amountOfMoves = 0;
        return moves;
    }
    uint8_t currentBoard = board->AS.currentBoard;
    if (currentBoard == ANY_BOARD) {
        *amountOfMoves = generateMovesAnyBoard(board, moves);
        return moves;
    }
    return getMovesSingleBoard(board, currentBoard, amountOfMoves);
}


void makeRandomTemporaryMove(Board* board, pcg32_random_t* rng) {
    uint8_t currentBoard = board->AS.currentBoard;
    uint8_t randomMoveIndex;
    if (currentBoard == ANY_BOARD) {
        uint8_t randomMultipleBoardMoveIndex = pcg32_boundedrand_r(rng, board->AS.totalAmountOfOpenSquares);
        int boardIndex = 0;
        int cumulativeSum = 0;
        while (boardIndex < 9 && cumulativeSum <= randomMultipleBoardMoveIndex) {
            cumulativeSum += board->AS.amountOfOpenSquaresBySmallBoard[boardIndex++];
        }
        cumulativeSum -= board->AS.amountOfOpenSquaresBySmallBoard[--boardIndex];
        randomMoveIndex = randomMultipleBoardMoveIndex - cumulativeSum;
        currentBoard = boardIndex;
    } else {
        randomMoveIndex = pcg32_boundedrand_r(rng, board->AS.amountOfOpenSquaresBySmallBoard[currentBoard]);
    }
    int8_t amountOfMoves;
    Square* moves = getMovesSingleBoard(board, currentBoard, &amountOfMoves);
    Square randomMove = moves[randomMoveIndex];
    makeTemporaryMove(board, randomMove);
}


bool nextBoardIsEmpty(Board* board) {
    uint8_t currentBoard = board->AS.currentBoard;
    return currentBoard != ANY_BOARD
           && (getSmallBoard(board->player1, currentBoard) | getSmallBoard(board->player2, currentBoard)) == 0;
}


uint8_t getCurrentBoard(Board* board) {
    return board->AS.currentBoard;
}


void revertToCheckpoint(Board* board) {
    revertToPlayerCheckpoint(board->player1);
    revertToPlayerCheckpoint(board->player2);
    board->AS = board->ASCheckpoint;
}


void updateCheckpoint(Board* board) {
    updatePlayerCheckpoint(board->player1);
    updatePlayerCheckpoint(board->player2);
    board->ASCheckpoint = board->AS;
}


Winner getSmallBoardWinner(Board* board, uint8_t boardIndex) {
    bool player1Bit = boardIsWon(board->player1, boardIndex);
    bool player2Bit = boardIsWon(board->player2, boardIndex);
    return 2*player2Bit + player1Bit;
}


Occupation getSquare(Board* board, Square square) {
    bool player1Bit = squareIsOccupied(board->player1, square);
    bool player2Bit = squareIsOccupied(board->player2, square);
    return 2*player2Bit + player1Bit;
}


uint8_t getNextBoard(Board* board, uint8_t previousPosition) {
    Winner smallBoardWinner = getSmallBoardWinner(board, previousPosition);
    return smallBoardWinner == NONE ? previousPosition : ANY_BOARD;
}


Winner calculateWinner(Board* board) {
    uint16_t player1BigBoard = getBigBoard(board->player1);
    uint16_t player2BigBoard = getBigBoard(board->player2);
    uint16_t decisiveBoards = player1BigBoard ^ player2BigBoard;
    uint16_t boardsWonByPlayer1 = player1BigBoard & decisiveBoards;
    if (isWin(boardsWonByPlayer1)) {
        return WIN_P1;
    }
    uint16_t boardsWonByPlayer2 = player2BigBoard & decisiveBoards;
    if (isWin(boardsWonByPlayer2)) {
        return WIN_P2;
    }
    if ((player1BigBoard | player2BigBoard) == 511) {
        int player1AmountBoardsWon = __builtin_popcount(player1BigBoard);
        int player2AmountBoardsWon = __builtin_popcount(player2BigBoard);
        return player1AmountBoardsWon > player2AmountBoardsWon
               ? WIN_P1
               : player1AmountBoardsWon < player2AmountBoardsWon
                 ? WIN_P2
                 : DRAW;
    }
    return NONE;
}


void makeTemporaryMove(Board* board, Square square) {
    bool bigBoardWasUpdated = board->AS.currentPlayer == PLAYER1
                              ? setSquareOccupied(board->player1, board->player2, square)
                              : setSquareOccupied(board->player2, board->player1, square);
    if (bigBoardWasUpdated) {
        board->AS.winner = calculateWinner(board);
        board->AS.totalAmountOfOpenSquares -= board->AS.amountOfOpenSquaresBySmallBoard[square.board];
        board->AS.amountOfOpenSquaresBySmallBoard[square.board] = 0;
    } else {
        board->AS.totalAmountOfOpenSquares--;
        board->AS.amountOfOpenSquaresBySmallBoard[square.board]--;
    }
    board->AS.currentPlayer = otherPlayer(board->AS.currentPlayer);
    board->AS.currentBoard = getNextBoard(board, square.position);
    board->AS.ply++;
}


void makePermanentMove(Board* board, Square square) {
    makeTemporaryMove(board, square);
    updateCheckpoint(board);
}


Winner getWinner(Board* board) {
    return board->AS.winner;
}


void setMe(Board* board, Player player) {
    board->me = player;
    board->AS.ply = player == PLAYER2;
    board->ASCheckpoint.ply = player == PLAYER2;
}


bool currentPlayerIsMe(Board* board) {
    return board->AS.currentPlayer == board->me;
}


uint8_t getPly(Board* board) {
    return board->AS.ply;
}

// END BOARD















// START MCTS_NODE
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
    MCTSNode* root = calloc(1, sizeof(MCTSNode));
    root->amountOfChildren = -1;
    root->player = PLAYER2;
    root->square.board = 9;
    root->square.position = 9;
    root->amountOfUntriedMoves = -1;
    return root;
}


MCTSNode* createMCTSNode(MCTSNode* parent, Square square) {
    MCTSNode* node = calloc(1, sizeof(MCTSNode));
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
    free(node);
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
        node->untriedMoves = malloc(sizeof(Square));
        uint8_t currentBoard = getCurrentBoard(board);
        Square sameBoard = {currentBoard, currentBoard};
        node->untriedMoves[0] = sameBoard;
        return true;
    }
    if (getPly(board) == 0) {
        node->amountOfUntriedMoves = 1;
        node->untriedMoves = malloc(sizeof(Square));
        Square bestFirstMove = {4, 4};
        node->untriedMoves[0] = bestFirstMove;
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
            node->untriedMoves = malloc(amountOfMoves * sizeof(Square));
            for (int i = 0; i < amountOfMoves; i++) {
                node->untriedMoves[i] = moves[i];
            }
        }
    }
}


bool isLeafNode(MCTSNode* node, Board* board) {
    discoverChildNodes(node, board);
    return node->amountOfUntriedMoves > 0;
}


#define EXPLORATION_PARAMETER 0.5f
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
    node->children = realloc(node->children, (node->amountOfChildren + 1) * sizeof(MCTSNode*));
    node->children[node->amountOfChildren++] = newChild;
    return newChild;
}


MCTSNode* selectNextChild(MCTSNode* node, Board* board) {
    discoverChildNodes(node, board);
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
    return highestUCTChild;
}


MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square) {
    discoverChildNodes(root, board);
    MCTSNode* newRoot = NULL;
    for (int i = 0; i < root->amountOfChildren; i++) {
        MCTSNode* child = root->children[i];
        if (squaresAreEqual(square, child->square)) {
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
    return newRoot;
}


void backpropagate(MCTSNode* node, Winner winner) {
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
// END MCTS_NODE

















// START FIND_NEXT_MOVE
MCTSNode* selectLeaf(Board* board, MCTSNode* root) {
    MCTSNode* currentNode = root;
    while (!isLeafNode(currentNode, board) && getWinner(board) == NONE) {
        currentNode = selectNextChild(currentNode, board);
        visitNode(currentNode, board);
    }
    return currentNode;
}


MCTSNode* expandLeaf(Board* board, MCTSNode* leaf) {
    MCTSNode* nextChild = selectNextChild(leaf, board);
    visitNode(nextChild, board);
    return nextChild;
}


Winner simulate(Board* board, pcg32_random_t* rng) {
    while (getWinner(board) == NONE) {
        makeRandomTemporaryMove(board, rng);
    }
    Winner simulationWinner = getWinner(board);
    revertToCheckpoint(board);
    return simulationWinner;
}


clock_t getDeadline(double time) {
    return clock() + (clock_t)(time*CLOCKS_PER_SEC);
}


bool hasTimeRemaining(clock_t deadline) {
    return clock() < deadline;
}


int findNextMove(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime) {
    clock_t deadline = getDeadline(allocatedTime);
    int amountOfSimulations = 0;
    while (++amountOfSimulations % 128 != 0 || hasTimeRemaining(deadline)) {
        MCTSNode* leaf = selectLeaf(board, root);
        MCTSNode* playoutNode;
        Winner simulationWinner;
        Winner winner = getWinner(board);
        if (winner == NONE) {
            playoutNode = expandLeaf(board, leaf);
            simulationWinner = simulate(board, rng);
        } else {
            playoutNode = leaf;
            simulationWinner = winner;
            setNodeWinner(playoutNode, winner);
            revertToCheckpoint(board);
        }
        backpropagate(playoutNode, simulationWinner);
    }
    return amountOfSimulations;
}
// END FIND_NEXT_MOVE

















// START HANDLE_TURN
MCTSNode* handleEnemyTurn(Board* board, MCTSNode* root, Square enemyMove) {
    if (enemyMove.board == 9 && enemyMove.position == 9) {
        setMe(board, PLAYER1);
        return root;
    }
    MCTSNode* newRoot = updateRoot(root, board, enemyMove);
    makePermanentMove(board, enemyMove);
    return newRoot;
}


HandleTurnResult handleTurn(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime, Square enemyMove) {
    root = handleEnemyTurn(board, root, enemyMove);
    int amountOfSimulations = findNextMove(board, root, rng, allocatedTime);
    Square move = getMostPromisingMove(root, board);
    MCTSNode* newRoot = updateRoot(root, board, move);
    makePermanentMove(board, move);
    HandleTurnResult result = {move, newRoot, amountOfSimulations};
    return result;
}
// END HANDLE_TURN






















// START MAIN
void skipMovesInput(FILE* file) {
    int validActionCount;
    fscanf(file, "%d", &validActionCount);  // NOLINT(cert-err34-c)
    for (int i = 0; i < validActionCount; i++) {
        int row;
        int col;
        fscanf(file, "%d%d", &row, &col);  // NOLINT(cert-err34-c)
    }
}


void printMove(MCTSNode* root, Square bestMove, int amountOfSimulations) {
    Square s = toGameNotation(bestMove);
    uint8_t x = s.board;
    uint8_t y = s.position;
    double winrate = getWinrate(root);
    printf("%d %d %.4f %d\n", x, y, winrate, amountOfSimulations);
    fflush(stdout);
}


void playGame(FILE* file, double timePerMove) {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode();
    pcg32_random_t rng;
    pcg32_srandom_r(&rng, 69, 420);
    while (true) {
        int enemy_row;
        int enemy_col;
        int v = fscanf(file, "%d%d", &enemy_row, &enemy_col);  // NOLINT(cert-err34-c)
        if (v == EOF) {
            break;
        }
        skipMovesInput(file);
        Square enemyMoveGameNotation = {enemy_row, enemy_col};
        Square enemyMove = toOurNotation(enemyMoveGameNotation);
        HandleTurnResult result = handleTurn(board, root, &rng, timePerMove, enemyMove);
        root = result.newRoot;
        printMove(root, result.move, result.amountOfSimulations);
    }
    freeMCTSTree(root);
    freeBoard(board);
}


#define TIME 0.085
int main() {
    // runTests();
    playGame(stdin, TIME);
}
// END MAIN