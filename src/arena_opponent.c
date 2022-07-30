#pragma GCC optimize("Ofast", "unroll-loops", "omit-frame-pointer", "inline")
#pragma GCC option("arch=native", "tune=native", "no-zero-upper")
#pragma GCC target("rdrnd", "popcnt", "avx", "bmi2")


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include "square.h"



// START HEADERS
bool squaresAreEqual2(Square square1, Square square2);

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CHECK(a,b) ((a) & (1ULL<<(b)))
#define BIT_CHANGE(a,b,c) ((a) ^= (-(c) ^ (a)) & (1ULL << (b)))

Square toGameNotation2(Square square);

typedef struct RNG2 {
    uint64_t state;
    uint64_t inc;
} RNG2;

void seedRNG2(RNG2* rng, uint64_t init_state, uint64_t init_seq);

uint8_t generateBoundedRandomNumber2(RNG2* rng, uint8_t bound);

#define Player bool
#define PLAYER1 0
#define PLAYER2 1
#define OTHER_PLAYER(p) ((p)^1)

#define Winner uint8_t
#define NONE 0
#define WIN_P1 1
#define WIN_P2 2
#define DRAW 3


typedef struct PlayerBitBoard2 {
    uint16_t smallBoards[9];
    uint16_t bigBoard;
    uint16_t checkpointSmallBoards[9];
    uint16_t checkpointBigBoard;
} PlayerBitBoard2;

void initializePlayerBitBoard2(PlayerBitBoard2* playerBitBoard);

bool boardIsWon2(PlayerBitBoard2* playerBitBoard, uint8_t board);

bool isWin2(uint16_t smallBoard);

bool setSquareOccupied2(PlayerBitBoard2* playerBitBoard, PlayerBitBoard2* otherPlayerBitBoard, Square square);

void revertToPlayerCheckpoint2(PlayerBitBoard2* playerBitBoard);

void updatePlayerCheckpoint2(PlayerBitBoard2* playerBitBoard);

typedef struct RolloutState2 {
    uint16_t instantWinBoards[2];
    uint16_t instantWinSmallBoards[2];
} RolloutState2;

void initializeLookupTable2();

void initializeRolloutState2(RolloutState2* RS);

bool hasWinningMove2(RolloutState2* RS, uint8_t currentBoard, Player player);

void updateSmallBoardState2(RolloutState2* RS, uint8_t boardIndex, uint16_t smallBoard, uint16_t otherSmallBoard, Player player);

void updateBigBoardState2(RolloutState2* RS, uint16_t bigBoard, uint16_t otherBigBoard, Player player);

#define TOTAL_SMALL_SQUARES 81

typedef struct Board2 Board2;

Board2* createBoard2();

void freeBoard2(Board2* board);

Square* generateMoves2(Board2* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves);

void makeRandomTemporaryMove2(Board2* board, RolloutState2* RS, RNG2* rng);

bool nextBoardIsEmpty2(Board2* board);

uint8_t getCurrentBoard2(Board2* board);

Player getCurrentPlayer2(Board2* board);

void revertToCheckpoint2(Board2* board);

void makeTemporaryMove2(Board2* board, Square square);

void makePermanentMove2(Board2* board, Square square);

Winner getWinner2(Board2* board);

void setMe2(Board2* board, Player player);

bool currentPlayerIsMe2(Board2* board);

uint8_t getPly2(Board2* board);

typedef struct MCTSNode2 MCTSNode2;

MCTSNode2* createMCTSRootNode2();

void freeMCTSTree2(MCTSNode2* root);

bool isLeafNode2(MCTSNode2* node, Board2* board);

MCTSNode2* selectNextChild2(MCTSNode2* node);

MCTSNode2* updateRoot2(MCTSNode2* root, Board2* board, Square square);

void backpropagate2(MCTSNode2* node, Winner winner, Player player);

void visitNode2(MCTSNode2* node, Board2* board);

void setNodeWinner2(MCTSNode2* node, Winner winner, Player player);

Square getMostPromisingMove2(MCTSNode2* node);

float getWinrate2(MCTSNode2* node);

int findNextMove2(Board2* board, MCTSNode2* root, RNG2* rng, double allocatedTime);

typedef struct HandleTurnResult2 {
    Square move;
    MCTSNode2* newRoot;
    int amountOfSimulations;
} HandleTurnResult2;

HandleTurnResult2 handleTurn2(Board2* board, MCTSNode2* root, RNG2* rng, double allocatedTime, Square enemyMove);

// END HEADERS












// START SQUARE
bool squaresAreEqual2(Square square1, Square square2) {
    return square1.board == square2.board && square1.position == square2.position;
}
// END SQUARE














// START UTIL
Square toGameNotation2(Square square) {
    uint8_t row = (square.position / 3) + 3*(square.board / 3);
    uint8_t col = (square.position % 3) + 3*(square.board % 3);
    Square result = {row, col};
    return result;
}
// END UTIL












// START RANDOM
uint32_t pcg32_random_r2(RNG2* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}


void seedRNG2(RNG2* rng, uint64_t init_state, uint64_t init_seq) {
    rng->state = 0U;
    rng->inc = (init_seq << 1u) | 1u;
    pcg32_random_r2(rng);
    rng->state += init_state;
    pcg32_random_r2(rng);
}


uint8_t generateBoundedRandomNumber2(RNG2* rng, uint8_t bound) {
    return ((uint64_t)pcg32_random_r2(rng) * (uint64_t)bound) >> 32;
}
// END RANDOM









// START PLAYER_BIT_BOARD
bool precalculatedIsWin[512];
bool isWin_2(uint16_t smallBoard) {
    __m128i masks = _mm_setr_epi16(0x7, 0x38, 0x1c0, 0x49, 0x92, 0x124, 0x111, 0x54);
    __m128i v1 = _mm_set1_epi16(1);
    __m128i boards128 = _mm_set1_epi16((short) smallBoard);
    __m128i andResult = _mm_and_si128(masks, boards128);
    __m128i result = _mm_cmpeq_epi16(andResult, masks);
    return !_mm_test_all_zeros(result, v1);
}


bool isWin2(uint16_t smallBoard) {
    return precalculatedIsWin[smallBoard];
}


void initializePlayerBitBoard2(PlayerBitBoard2* playerBitBoard) {
    for (uint16_t i = 0; i < 512; i++) {
        precalculatedIsWin[i] = isWin_2(i);
    }
    memset(playerBitBoard, 0, sizeof(PlayerBitBoard2));
}


bool boardIsWon2(PlayerBitBoard2* playerBitBoard, uint8_t board) {
    return BIT_CHECK(playerBitBoard->bigBoard, board);
}


bool isDraw2(uint16_t smallBoard, uint16_t otherPlayerSmallBoard) {
    return (smallBoard | otherPlayerSmallBoard) == 511;
}


bool setSquareOccupied2(PlayerBitBoard2* playerBitBoard, PlayerBitBoard2* otherPlayerBitBoard, Square square) {
    BIT_SET(playerBitBoard->smallBoards[square.board], square.position);
    uint16_t smallBoard = playerBitBoard->smallBoards[square.board];
    uint16_t otherPlayerSmallBoard = otherPlayerBitBoard->smallBoards[square.board];
    if (isWin2(smallBoard)) {
        BIT_SET(playerBitBoard->bigBoard, square.board);
        return true;
    } if (isDraw2(smallBoard, otherPlayerSmallBoard)) {
        BIT_SET(playerBitBoard->bigBoard, square.board);
        BIT_SET(otherPlayerBitBoard->bigBoard, square.board);
        return true;
    }
    return false;
}


void revertToPlayerCheckpoint2(PlayerBitBoard2* playerBitBoard) {
    memcpy(playerBitBoard->smallBoards, playerBitBoard->checkpointSmallBoards, 9 * sizeof(uint16_t));
    playerBitBoard->bigBoard = playerBitBoard->checkpointBigBoard;
}


void updatePlayerCheckpoint2(PlayerBitBoard2* playerBitBoard) {
    memcpy(playerBitBoard->checkpointSmallBoards, playerBitBoard->smallBoards, 9 * sizeof(uint16_t));
    playerBitBoard->checkpointBigBoard = playerBitBoard->bigBoard;
}
// END PLAYER_BIT_BOARD









// START SMART_ROLLOUT
uint16_t precalculatedInstantWinBoards[512];
uint16_t calculateInstantWinBoards_2(uint16_t bigBoard) {
    uint16_t lineAlreadyFilled[8] = {0x7, 0x38, 0x1c0, 0x49, 0x92, 0x124, 0x111, 0x54};
    uint16_t hasInstantWinBoardMasks[24] = {
            3, 5, 6,
            24, 40, 48,
            192, 320, 384,
            9, 65, 72,
            18, 130, 144,
            36, 260, 288,
            17, 257, 272,
            20, 68, 80
    };
    uint16_t instantWinBoard[24] = {
            2, 1, 0,
            5, 4, 3,
            8, 7, 6,
            6, 3, 0,
            7, 4, 1,
            8, 5, 2,
            8, 4, 0,
            6, 4, 2
    };
    uint16_t result = 0;
    for (int i = 0; i < 24; i++) {
        uint16_t m1 = hasInstantWinBoardMasks[i];
        uint16_t m2 = lineAlreadyFilled[i/3];
        if ((bigBoard & m1) == m1 && (bigBoard & m2) != m2) {
            BIT_SET(result, instantWinBoard[i]);
        }
    }
    return result;
}


uint16_t calculateInstantWinBoards2(uint16_t bigBoard, uint16_t otherPlayerBigBoard) {
    return precalculatedInstantWinBoards[bigBoard] & ~otherPlayerBigBoard;
}


bool smallBoardHasInstantWinMove2(uint16_t smallBoard, uint16_t otherSmallBoard) {
    return (precalculatedInstantWinBoards[smallBoard] & ~otherSmallBoard) != 0;
}


void initializeLookupTable2() {
    for (uint16_t bigBoard = 0; bigBoard < 512; bigBoard++) {
        precalculatedInstantWinBoards[bigBoard] = calculateInstantWinBoards_2(bigBoard);
    }
}


void initializeRolloutState2(RolloutState2* RS) {
    memset(RS, 0, sizeof(RolloutState2));
}


bool hasWinningMove2(RolloutState2* RS, uint8_t currentBoard, Player player) {
    uint16_t smallBoardsWithWinningMove = RS->instantWinBoards[player] & RS->instantWinSmallBoards[player];
    return (currentBoard == 9 && smallBoardsWithWinningMove) || BIT_CHECK(smallBoardsWithWinningMove, currentBoard);
}


void updateSmallBoardState2(RolloutState2* RS, uint8_t boardIndex, uint16_t smallBoard, uint16_t otherSmallBoard, Player player) {
    BIT_CHANGE(RS->instantWinSmallBoards[player], boardIndex, smallBoardHasInstantWinMove2(smallBoard, otherSmallBoard));
    BIT_CHANGE(RS->instantWinSmallBoards[OTHER_PLAYER(player)], boardIndex,
               smallBoardHasInstantWinMove2(otherSmallBoard, smallBoard));
}


void updateBigBoardState2(RolloutState2* RS, uint16_t bigBoard, uint16_t otherBigBoard, Player player) {
    RS->instantWinBoards[player] = calculateInstantWinBoards2(bigBoard, otherBigBoard);
    RS->instantWinBoards[OTHER_PLAYER(player)] = calculateInstantWinBoards2(otherBigBoard, bigBoard);
}
// END SMART_ROLLOUT
















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


typedef struct Board2 {
    PlayerBitBoard2 player1;
    PlayerBitBoard2 player2;
    AdditionalState AS;
    AdditionalState ASCheckpoint;
    Player me;
} Board2;


Square openSquares[512][9][9];
int8_t amountOfOpenSquares[512];
Winner winnerByBigBoards[512][512];


int8_t setOpenSquares2(Square openSquares_[9], uint8_t boardIndex, uint16_t bitBoard) {
    int8_t amountOfMoves = 0;
    while (bitBoard) {
        Square square = {boardIndex, __builtin_ffs(bitBoard) - 1};
        openSquares_[amountOfMoves++] = square;
        bitBoard &= bitBoard - 1;
    }
    return amountOfMoves;
}


Winner calculateWinner2(uint16_t player1BigBoard, uint16_t player2BigBoard) {
    uint16_t decisiveBoards = player1BigBoard ^ player2BigBoard;
    uint16_t boardsWonByPlayer1 = player1BigBoard & decisiveBoards;
    if (isWin2(boardsWonByPlayer1)) {
        return WIN_P1;
    }
    uint16_t boardsWonByPlayer2 = player2BigBoard & decisiveBoards;
    if (isWin2(boardsWonByPlayer2)) {
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


Board2* createBoard2() {
    Board2* board = malloc(sizeof(Board2));
    initializePlayerBitBoard2(&board->player1);
    initializePlayerBitBoard2(&board->player2);
    board->AS.currentPlayer = PLAYER1;
    board->AS.currentBoard = ANY_BOARD;
    board->AS.winner = NONE;
    board->AS.ply = 0;
    board->AS.totalAmountOfOpenSquares = 81;
    for (int boardIndex = 0; boardIndex < 9; boardIndex++) {
        board->AS.amountOfOpenSquaresBySmallBoard[boardIndex] = 9;
    }
    board->ASCheckpoint = board->AS;
    for (int boardIndex = 0; boardIndex < 9; boardIndex++) {
        for (int bitBoard = 0; bitBoard < 512; bitBoard++) {
            amountOfOpenSquares[bitBoard] =
                    setOpenSquares2(openSquares[bitBoard][boardIndex], boardIndex, bitBoard);
        }
    }
    for (uint16_t player1BigBoard = 0; player1BigBoard < 512; player1BigBoard++) {
        for (uint16_t player2BigBoard = 0; player2BigBoard < 512; player2BigBoard++) {
            Winner winner = calculateWinner2(player1BigBoard, player2BigBoard);
            winnerByBigBoards[player1BigBoard][player2BigBoard] = winner;
        }
    }
    board->me = PLAYER2;
    return board;
}


void freeBoard2(Board2* board) {
    free(board);
}


Square* getMovesSingleBoard2(Board2* board, uint8_t boardIndex, int8_t* amountOfMoves) {
    uint16_t bitBoard = ~(board->player1.smallBoards[boardIndex] | board->player2.smallBoards[boardIndex]) & 511;
    *amountOfMoves = amountOfOpenSquares[bitBoard];
    return openSquares[bitBoard][boardIndex];
}


int8_t copyMovesSingleBoard2(Board2* board, uint8_t boardIndex, Square moves[TOTAL_SMALL_SQUARES], int8_t amountOfMoves) {
    uint16_t bitBoard = ~(board->player1.smallBoards[boardIndex] | board->player2.smallBoards[boardIndex]) & 511;
    memcpy(&moves[amountOfMoves], openSquares[bitBoard][boardIndex],
           amountOfOpenSquares[bitBoard] * sizeof(Square));
    return (int8_t)(amountOfMoves + amountOfOpenSquares[bitBoard]);
}


int8_t generateMovesAnyBoard2(Board2* board, Square moves[TOTAL_SMALL_SQUARES]) {
    int8_t amountOfMoves = 0;
    uint16_t undecidedSmallBoards = ~(board->player1.bigBoard | board->player2.bigBoard) & 511;
    while (undecidedSmallBoards) {
        uint8_t boardIndex = __builtin_ffs(undecidedSmallBoards) - 1;
        amountOfMoves = copyMovesSingleBoard2(board, boardIndex, moves, amountOfMoves);
        undecidedSmallBoards &= undecidedSmallBoards - 1;
    }
    return amountOfMoves;
}


Square* generateMoves2(Board2* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves) {
    if (board->AS.winner != NONE) {
        *amountOfMoves = 0;
        return moves;
    }
    uint8_t currentBoard = board->AS.currentBoard;
    if (currentBoard == ANY_BOARD) {
        *amountOfMoves = generateMovesAnyBoard2(board, moves);
        return moves;
    }
    return getMovesSingleBoard2(board, currentBoard, amountOfMoves);
}


uint8_t getNextBoard2(Board2* board, uint8_t previousPosition) {
    bool smallBoardIsDecided = boardIsWon2(&board->player1, previousPosition) ||
                               boardIsWon2(&board->player2, previousPosition);
    return smallBoardIsDecided ? ANY_BOARD : previousPosition;
}


void makeRolloutTemporaryMove2(Board2* board, RolloutState2* RS, Square square) {
    PlayerBitBoard2* playerBitBoard = &board->player1 + board->AS.currentPlayer;
    PlayerBitBoard2* otherPlayerBitBoard = &board->player1 + !board->AS.currentPlayer;
    if (setSquareOccupied2(playerBitBoard, otherPlayerBitBoard, square)) {
        board->AS.winner = winnerByBigBoards[board->player1.bigBoard][board->player2.bigBoard];
        board->AS.totalAmountOfOpenSquares -= board->AS.amountOfOpenSquaresBySmallBoard[square.board];
        board->AS.amountOfOpenSquaresBySmallBoard[square.board] = 0;
        updateBigBoardState2(RS, playerBitBoard->bigBoard, otherPlayerBitBoard->bigBoard, board->AS.currentPlayer);
    } else {
        board->AS.totalAmountOfOpenSquares--;
        board->AS.amountOfOpenSquaresBySmallBoard[square.board]--;
    }
    updateSmallBoardState2(RS, square.board, playerBitBoard->smallBoards[square.board],
                           otherPlayerBitBoard->smallBoards[square.board], board->AS.currentPlayer);
    board->AS.currentPlayer ^= 1;
    board->AS.currentBoard = getNextBoard2(board, square.position);
    board->AS.ply++;
}


void makeRandomTemporaryMove2(Board2* board, RolloutState2* RS, RNG2* rng) {
    uint8_t currentBoard = board->AS.currentBoard;
    if (hasWinningMove2(RS, currentBoard, board->AS.currentPlayer)) {
        board->AS.winner = board->AS.currentPlayer + 1;
        return;
    }
    uint8_t randomMoveIndex;
    if (currentBoard == ANY_BOARD) {
        randomMoveIndex = generateBoundedRandomNumber2(rng, board->AS.totalAmountOfOpenSquares);
        currentBoard = 0;
        while (currentBoard < 9 && randomMoveIndex < 128) {
            randomMoveIndex -= board->AS.amountOfOpenSquaresBySmallBoard[currentBoard++];
        }
        randomMoveIndex += board->AS.amountOfOpenSquaresBySmallBoard[--currentBoard];
    } else {
        randomMoveIndex = generateBoundedRandomNumber2(rng, board->AS.amountOfOpenSquaresBySmallBoard[currentBoard]);
    }
    uint16_t bitBoard = ~(board->player1.smallBoards[currentBoard] | board->player2.smallBoards[currentBoard]) & 511;
    makeRolloutTemporaryMove2(board, RS, openSquares[bitBoard][currentBoard][randomMoveIndex]);
}


bool nextBoardIsEmpty2(Board2* board) {
    uint8_t currentBoard = board->AS.currentBoard;
    return currentBoard != ANY_BOARD
           && (board->player1.smallBoards[currentBoard] | board->player2.smallBoards[currentBoard]) == 0;
}


uint8_t getCurrentBoard2(Board2* board) {
    return board->AS.currentBoard;
}


Player getCurrentPlayer2(Board2* board) {
    return board->AS.currentPlayer;
}


void revertToCheckpoint2(Board2* board) {
    revertToPlayerCheckpoint2(&board->player1);
    revertToPlayerCheckpoint2(&board->player2);
    board->AS = board->ASCheckpoint;
}


void updateCheckpoint2(Board2* board) {
    updatePlayerCheckpoint2(&board->player1);
    updatePlayerCheckpoint2(&board->player2);
    board->ASCheckpoint = board->AS;
}


void makeTemporaryMove2(Board2* board, Square square) {
    PlayerBitBoard2* p1 = &board->player1;
    if (setSquareOccupied2(p1 + board->AS.currentPlayer, p1 + !board->AS.currentPlayer, square)) {
        board->AS.winner = winnerByBigBoards[board->player1.bigBoard][board->player2.bigBoard];
        board->AS.totalAmountOfOpenSquares -= board->AS.amountOfOpenSquaresBySmallBoard[square.board];
        board->AS.amountOfOpenSquaresBySmallBoard[square.board] = 0;
    } else {
        board->AS.totalAmountOfOpenSquares--;
        board->AS.amountOfOpenSquaresBySmallBoard[square.board]--;
    }
    board->AS.currentPlayer ^= 1;
    board->AS.currentBoard = getNextBoard2(board, square.position);
    board->AS.ply++;
}


void makePermanentMove2(Board2* board, Square square) {
    makeTemporaryMove2(board, square);
    updateCheckpoint2(board);
}


Winner getWinner2(Board2* board) {
    return board->AS.winner;
}


void setMe2(Board2* board, Player player) {
    board->me = player;
}


bool currentPlayerIsMe2(Board2* board) {
    return board->AS.currentPlayer == board->me;
}


uint8_t getPly2(Board2* board) {
    return board->AS.ply;
}
// END BOARD















// START MCTS_NODE
typedef struct MCTSNode2 {
    MCTSNode2* parent;
    MCTSNode2* children;
    float wins;
    float sims;
    float simsInverted;
    Square square;
    int8_t amountOfChildren;
    int8_t amountOfUntriedMoves;
} MCTSNode2;


MCTSNode2* createMCTSRootNode2() {
    MCTSNode2* root = calloc(1, sizeof(MCTSNode2));
    root->amountOfChildren = -1;
    root->square.board = 9;
    root->square.position = 9;
    root->amountOfUntriedMoves = -1;
    return root;
}


void initializeMCTSNode2(MCTSNode2* parent, Square square, MCTSNode2* node) {
    node->parent = parent;
    node->children = NULL;
    node->wins = 0.0f;
    node->sims = 0.0f;
    node->simsInverted = 0.0f;
    node->square = square;
    node->amountOfChildren = -1;
    node->amountOfUntriedMoves = -1;
}


MCTSNode2* copyMCTSNode2(MCTSNode2* original) {
    MCTSNode2* copy = malloc(sizeof(MCTSNode2));
    copy->parent = original->parent;
    copy->children = original->children;
    copy->wins = original->wins;
    copy->sims = original->sims;
    copy->simsInverted = original->simsInverted;
    copy->square = original->square;
    copy->amountOfChildren = original->amountOfChildren;
    copy->amountOfUntriedMoves = original->amountOfUntriedMoves;
    for (int i = 0; i < copy->amountOfChildren; i++) {
        copy->children[i].parent = copy;
    }
    return copy;
}


void freeMCTSTree2(MCTSNode2* root) {
    for (int i = 0; i < root->amountOfChildren; i++) {
        freeMCTSTree2(&root->children[i]);
    }
    free(root->children);
}


void singleChild2(MCTSNode2* node, Square square) {
    node->amountOfUntriedMoves = 1;
    node->children = malloc(sizeof(MCTSNode2));
    node->children[0].square = square;
}


bool handleSpecialCases2(MCTSNode2* node, Board2* board) {
    if (nextBoardIsEmpty2(board) && currentPlayerIsMe2(board) && getPly2(board) <= 20) {
        uint8_t currentBoard = getCurrentBoard2(board);
        Square sameBoard = {currentBoard, currentBoard};
        singleChild2(node, sameBoard);
        return true;
    }
    if (currentPlayerIsMe2(board) && getPly2(board) == 0) {
        Square bestFirstMove = {4, 4};
        singleChild2(node, bestFirstMove);
        return true;
    }
    return false;
}


void discoverChildNodes2(MCTSNode2* node, Board2* board) {
    if (node->amountOfChildren == -1) {
        node->amountOfChildren = 0;
        if (!handleSpecialCases2(node, board)) {
            Square movesArray[TOTAL_SMALL_SQUARES];
            int8_t amountOfMoves;
            Square* moves = generateMoves2(board, movesArray, &amountOfMoves);
            node->amountOfUntriedMoves = amountOfMoves;
            node->children = malloc(amountOfMoves * sizeof(MCTSNode2));
            for (int i = 0; i < amountOfMoves; i++) {
                node->children[i].square = moves[i];
            }
        }
    }
}


bool isLeafNode2(MCTSNode2* node, Board2* board) {
    discoverChildNodes2(node, board);
    return node->amountOfUntriedMoves > 0;
}


void fastSquareRoot2(float* restrict pOut, float* restrict pIn) {
    __m128 in = _mm_load_ss(pIn);
    _mm_store_ss(pOut, _mm_mul_ss(in, _mm_rsqrt_ss(in)));
}


#define EXPLORATION_PARAMETER 0.459375f
float getUCTValue2(MCTSNode2* node, float parentLogSims) {
    float c = EXPLORATION_PARAMETER;
    float sqrtIn = parentLogSims * node->simsInverted;
    float sqrtOut;
    fastSquareRoot2(&sqrtOut, &sqrtIn);
    return node->wins*node->simsInverted + c*sqrtOut;
}


MCTSNode2* expandNode2(MCTSNode2* node, int childIndex) {
    MCTSNode2* newChild = &node->children[childIndex + node->amountOfChildren++];
    initializeMCTSNode2(node, newChild->square, newChild);
    node->amountOfUntriedMoves--;
    return newChild;
}


// From: https://github.com/etheory/fastapprox/blob/master/fastapprox/src/fastlog.h
float fastLog22(float x) {
    union { float f; uint32_t i; } vx = { x };
    float y = (float)vx.i;
    y *= 1.1920928955078125e-7f;
    return y - 126.94269504f;
}


MCTSNode2* selectNextChild2(MCTSNode2* node) {
    if (node->amountOfUntriedMoves) {
        return expandNode2(node, 0);
    }
    float logSims = fastLog22(node->sims);
    MCTSNode2* highestUCTChild = &node->children[0];
    float highestUCT = getUCTValue2(highestUCTChild, logSims);
    for (int i = 1; i < node->amountOfChildren; i++) {
        MCTSNode2* child = &node->children[i];
        float UCT = getUCTValue2(child, logSims);
        if (UCT > highestUCT) {
            highestUCTChild = child;
            highestUCT = UCT;
        }
    }
    return highestUCTChild;
}


MCTSNode2* updateRoot2(MCTSNode2* root, Board2* board, Square square) {
    discoverChildNodes2(root, board);
    MCTSNode2* newRoot = NULL;
    for (int i = 0; i < root->amountOfChildren; i++) {
        MCTSNode2* child = &root->children[i];
        if (squaresAreEqual2(square, child->square)) {
            newRoot = child;
        } else {
            freeMCTSTree2(child);
        }
    }
    if (newRoot == NULL) {
        for (int i = 0; i < root->amountOfUntriedMoves; i++) {
            if (squaresAreEqual2(square, root->children[root->amountOfChildren + i].square)) {
                newRoot = expandNode2(root, i);
                break;
            }
        }
    }
    newRoot->parent = NULL;
    newRoot = copyMCTSNode2(newRoot);
    free(root->children);
    return newRoot;
}


void backpropagate2(MCTSNode2* node, Winner winner, Player player) {
    MCTSNode2* currentNode = node;
    float reward = winner == DRAW? 0.5f : player + 1 == winner? 1.0f : 0.0f;
    while (currentNode != NULL) {
        currentNode->wins += reward;
        currentNode->simsInverted = 1.0f / ++currentNode->sims;
        reward = 1 - reward;
        currentNode = currentNode->parent;
    }
}


void visitNode2(MCTSNode2* node, Board2* board) {
    makeTemporaryMove2(board, node->square);
}


#define A_LOT 100000.0f
void setNodeWinner2(MCTSNode2* node, Winner winner, Player player) {
    if (winner != DRAW) {
        bool win = player + 1 == winner;
        node->wins += win? A_LOT : -A_LOT;
        if (!win) {
            node->parent->wins += A_LOT;
        }
    }
}


Square getMostPromisingMove2(MCTSNode2* node) {
    MCTSNode2* highestSimsChild = &node->children[0];
    float highestSims = highestSimsChild->sims;
    for (int i = 1; i < node->amountOfChildren; i++) {
        MCTSNode2* child = &node->children[i];
        float sims = child->sims;
        if (sims > highestSims) {
            highestSimsChild = child;
            highestSims = sims;
        }
    }
    return highestSimsChild->square;
}


float getWinrate2(MCTSNode2* node) {
    return node->wins * node->simsInverted;
}
// END MCTS_NODE

















// START FIND_NEXT_MOVE
MCTSNode2* selectLeaf2(Board2* board, MCTSNode2* root) {
    MCTSNode2* currentNode = root;
    while (!isLeafNode2(currentNode, board) && getWinner2(board) == NONE) {
        currentNode = selectNextChild2(currentNode);
        visitNode2(currentNode, board);
    }
    return currentNode;
}


MCTSNode2* expandLeaf2(Board2* board, MCTSNode2* leaf) {
    MCTSNode2* nextChild = selectNextChild2(leaf);
    visitNode2(nextChild, board);
    return nextChild;
}


Winner simulate2(Board2* board, RNG2* rng) {
    RolloutState2 RS;
    initializeRolloutState2(&RS);
    while (getWinner2(board) == NONE) {
        makeRandomTemporaryMove2(board, &RS, rng);
    }
    return getWinner2(board);
}


bool hasTimeRemaining2(struct timeval start, double allocatedTime) {
    struct timeval end;
    gettimeofday(&end, NULL);
    double timePassed = (double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec);
    return timePassed < allocatedTime;
}


int findNextMove2(Board2* board, MCTSNode2* root, RNG2* rng, double allocatedTime) {
    int amountOfSimulations = 0;
    struct timeval start;
    gettimeofday(&start, NULL);
    while (++amountOfSimulations % 128 != 0 || hasTimeRemaining2(start, allocatedTime)) {
        MCTSNode2* leaf = selectLeaf2(board, root);
        MCTSNode2* playoutNode;
        Winner simulationWinner;
        Winner winner = getWinner2(board);
        Player player;
        if (winner == NONE) {
            playoutNode = expandLeaf2(board, leaf);
            player = OTHER_PLAYER(getCurrentPlayer2(board));
            simulationWinner = simulate2(board, rng);
        } else {
            playoutNode = leaf;
            simulationWinner = winner;
            player = OTHER_PLAYER(getCurrentPlayer2(board));
            setNodeWinner2(playoutNode, winner, player);
        }
        backpropagate2(playoutNode, simulationWinner, player);
        revertToCheckpoint2(board);
    }
    return amountOfSimulations;
}
// END FIND_NEXT_MOVE

















// START HANDLE_TURN
MCTSNode2* handleEnemyTurn2(Board2* board, MCTSNode2* root, Square enemyMove) {
    if (enemyMove.board == 9 && enemyMove.position == 9) {
        setMe2(board, PLAYER1);
        return root;
    }
    MCTSNode2* newRoot = updateRoot2(root, board, enemyMove);
    makePermanentMove2(board, enemyMove);
    return newRoot;
}


HandleTurnResult2 handleTurn2(Board2* board, MCTSNode2* root, RNG2* rng, double allocatedTime, Square enemyMove) {
    root = handleEnemyTurn2(board, root, enemyMove);
    int amountOfSimulations = findNextMove2(board, root, rng, allocatedTime);
    Square move = getMostPromisingMove2(root);
    MCTSNode2* newRoot = updateRoot2(root, board, move);
    makePermanentMove2(board, move);
    HandleTurnResult2 result = {move, newRoot, amountOfSimulations};
    return result;
}
// END HANDLE_TURN






















// START MAIN
typedef struct StateOpponent {
    Board2* board;
    MCTSNode2* root;
    RNG2 rng;
} StateOpponent;


StateOpponent* initializeStateOpponent() {
    StateOpponent* state = malloc(sizeof(StateOpponent));
    state->board = createBoard2();
    state->root = createMCTSRootNode2();
    seedRNG2(&state->rng, 69, 420);
    return state;
}


void freeStateOpponent(StateOpponent* stateOpponent) {
    if (stateOpponent == NULL) {
        return;
    }
    freeBoard2(stateOpponent->board);
    freeMCTSTree2(stateOpponent->root);
    free(stateOpponent);
}


Square playTurnOpponent(StateOpponent** state, double allocatedTime, Square enemyMove) {
    initializeLookupTable2();
    HandleTurnResult2 result = handleTurn2((*state)->board, (*state)->root, &(*state)->rng, allocatedTime, enemyMove);
    (*state)->root = result.newRoot;
    if (getWinner2((*state)->board) != NONE) {
        freeStateOpponent(*state);
        *state = NULL;
    }
    return result.move;
}
// END MAIN