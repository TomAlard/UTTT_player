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



// START HEADERS

typedef struct Square2 {
    uint8_t board;
    uint8_t position;
} Square2;

bool squaresAreEqual2(Square2 square1, Square2 square2);

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CHECK(a,b) ((a) & (1ULL<<(b)))
#define BIT_CHANGE(a,b,c) ((a) = (a) & ~(b) | (-(c) & (b)))
#define ONE_BIT_SET(a) (((a) & ((a)-1)) == 0 && (a) != 0)

typedef struct RNG2 {
    uint64_t state;
    uint64_t inc;
} RNG2;

void seedRNG2(RNG2* rng, uint64_t init_state, uint64_t init_seq);

uint8_t generateBoundedRandomNumber2(RNG2* rng, uint8_t bound);

void shuffle2(int* array, int n, RNG2* rng);

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
} PlayerBitBoard2;

void initializePlayerBitBoard2(PlayerBitBoard2* playerBitBoard);

bool isWin2(uint16_t smallBoard);

bool setSquareOccupied2(PlayerBitBoard2* playerBitBoard, PlayerBitBoard2* otherPlayerBitBoard, Square2 square);

#define TOTAL_SMALL_SQUARES 81
#define ANY_BOARD 9

typedef struct State2 {
    PlayerBitBoard2 player1;
    PlayerBitBoard2 player2;
    uint16_t instantWinBoards[2];
    uint16_t instantWinSmallBoards[2];
    Player currentPlayer;
    uint8_t currentBoard;
    Winner winner;
    uint8_t ply;
    uint8_t totalAmountOfOpenSquares;
    uint8_t amountOfOpenSquaresBySmallBoard[9];
} State2;

typedef struct Board2 {
    State2 state;
    State2 stateCheckpoint;
    Player me;
} Board2;

Square2 openSquares2[512][9][9];
int8_t amountOfOpenSquares2[512];
Winner winnerByBigBoards2[512][512];

Board2* createBoard2();

void freeBoard2(Board2* board);

Square2* generateMoves2(Board2* board, Square2 moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves);

uint8_t getNextBoard2(Board2* board, uint8_t previousPosition);

bool nextBoardIsEmpty2(Board2* board);

bool nextBoardHasOneMoveFromBothPlayers2(Board2* board);

uint8_t getCurrentBoard2(Board2* board);

Player getCurrentPlayer2(Board2* board);

void revertToCheckpoint2(Board2* board);

void makeTemporaryMove2(Board2* board, Square2 square);

void makePermanentMove2(Board2* board, Square2 square);

Winner getWinnerAfterMove2(Board2* board, Square2 square);

Winner getWinner2(Board2* board);

void setMe2(Board2* board, Player player);

bool currentPlayerIsMe2(Board2* board);

uint8_t getPly2(Board2* board);

void initializeLookupTable2();

bool hasWinningMove2(Board2* board, uint16_t smallBoardsWithWinningMove);

void updateSmallBoardState2(Board2* board, uint8_t boardIndex);

void updateBigBoardState2(Board2* board);

void makeRandomTemporaryMove2(Board2* board, RNG2* rng);

Winner rollout2(Board2* board, RNG2* rng);

typedef struct MCTSNode2 {
    struct MCTSNode2* parent;
    struct MCTSNode2* children;
    float wins;
    float sims;
    float simsInverted;
    Square2 square;
    int8_t amountOfChildren;
    int8_t amountOfUntriedMoves;
} MCTSNode2;

MCTSNode2* createMCTSRootNode2();

void freeMCTSTree2(MCTSNode2* root);

void discoverChildNodes2(MCTSNode2* node, Board2* board, RNG2* rng);

bool isLeafNode2(MCTSNode2* node, Board2* board, RNG2* rng);

MCTSNode2* selectNextChild2(MCTSNode2* node);

MCTSNode2* expandNextChild2(MCTSNode2* node);

MCTSNode2* updateRoot2(MCTSNode2* root, Board2* board, Square2 square);

void backpropagate2(MCTSNode2* node, Winner winner, Player player);

void visitNode2(MCTSNode2* node, Board2* board);

Square2 getMostPromisingMove2(MCTSNode2* node);

float getWinrate2(MCTSNode2* node);

void initializePriorsLookupTable2();

void applyPriors2(Board2* board, MCTSNode2* parent);

void setNodeWinner2(MCTSNode2* node, Winner winner, Player player);

int findNextMove2(Board2* board, MCTSNode2* root, RNG2* rng, double allocatedTime);

typedef struct HandleTurnResult2 {
    Square2 move;
    MCTSNode2* newRoot;
    int amountOfSimulations;
} HandleTurnResult2;

HandleTurnResult2 handleTurn2(Board2* board, MCTSNode2* root, RNG2* rng, double allocatedTime, Square2 enemyMove);

// END HEADERS












// START SQUARE
bool squaresAreEqual2(Square2 square1, Square2 square2) {
    return square1.board == square2.board && square1.position == square2.position;
}
// END SQUARE












// START RANDOM
uint32_t generateRandomNumber2(RNG2* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}


void seedRNG2(RNG2* rng, uint64_t init_state, uint64_t init_seq) {
    rng->state = 0U;
    rng->inc = (init_seq << 1u) | 1u;
    generateRandomNumber2(rng);
    rng->state += init_state;
    generateRandomNumber2(rng);
}


uint8_t generateBoundedRandomNumber2(RNG2* rng, uint8_t bound) {
    return ((uint64_t) generateRandomNumber2(rng) * (uint64_t)bound) >> 32;
}


void shuffle2(int* array, int n, RNG2* rng) {
    uint32_t maxRandomNumber = (1L << 32) - 1;
    for (int i = 0; i < n - 1; i++) {
        uint32_t j = i + generateRandomNumber2(rng) / (maxRandomNumber / (n - i) + 1);
        int t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}
// END RANDOM









// START PLAYER_BIT_BOARD
bool isWin2(uint16_t smallBoard) {
    __m128i masks = _mm_setr_epi16(0x7, 0x38, 0x1c0, 0x49, 0x92, 0x124, 0x111, 0x54);
    __m128i v1 = _mm_set1_epi16(1);
    __m128i boards128 = _mm_set1_epi16((short) smallBoard);
    __m128i andResult = _mm_and_si128(masks, boards128);
    __m128i result = _mm_cmpeq_epi16(andResult, masks);
    return !_mm_test_all_zeros(result, v1);
}


void initializePlayerBitBoard2(PlayerBitBoard2* playerBitBoard) {
    memset(playerBitBoard, 0, sizeof(PlayerBitBoard2));
}


bool isDraw2(uint16_t smallBoard, uint16_t otherPlayerSmallBoard) {
    return (smallBoard | otherPlayerSmallBoard) == 511;
}


bool setSquareOccupied2(PlayerBitBoard2* playerBitBoard, PlayerBitBoard2* otherPlayerBitBoard, Square2 square) {
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
// END PLAYER_BIT_BOARD









// START SMART_ROLLOUT
uint16_t precalculatedInstantWinBoards2[512];
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
    return precalculatedInstantWinBoards2[bigBoard & ~otherPlayerBigBoard] & ~otherPlayerBigBoard;
}


bool smallBoardHasInstantWinMove2(uint16_t smallBoard, uint16_t otherSmallBoard) {
    return calculateInstantWinBoards2(smallBoard, otherSmallBoard) != 0;
}


void initializeLookupTable2() {
    for (uint16_t bigBoard = 0; bigBoard < 512; bigBoard++) {
        precalculatedInstantWinBoards2[bigBoard] = calculateInstantWinBoards_2(bigBoard);
    }
}


bool hasWinningMove2(Board2* board, uint16_t smallBoardsWithWinningMove) {
    uint8_t currentBoard = board->state.currentBoard;
    return (currentBoard == ANY_BOARD && smallBoardsWithWinningMove)
           || BIT_CHECK(smallBoardsWithWinningMove, currentBoard);
}


void updateSmallBoardState2(Board2* board, uint8_t boardIndex) {
    uint16_t p1 = board->state.player1.smallBoards[boardIndex];
    uint16_t p2 = board->state.player2.smallBoards[boardIndex];
    uint16_t mask = 1ULL << boardIndex;
    BIT_CHANGE(board->state.instantWinSmallBoards[PLAYER1], mask, smallBoardHasInstantWinMove2(p1, p2));
    BIT_CHANGE(board->state.instantWinSmallBoards[PLAYER2], mask, smallBoardHasInstantWinMove2(p2, p1));
}


void updateBigBoardState2(Board2* board) {
    uint16_t p1 = board->state.player1.bigBoard;
    uint16_t p2 = board->state.player2.bigBoard;
    board->state.instantWinBoards[PLAYER1] = calculateInstantWinBoards2(p1, p2);
    board->state.instantWinBoards[PLAYER2] = calculateInstantWinBoards2(p2, p1);
}
// END SMART_ROLLOUT










// START ROLLOUT
Square2 generateMove2(Board2* board, RNG2* rng) {
    uint8_t currentBoard = board->state.currentBoard;
    uint8_t randomMoveIndex;
    if (currentBoard == ANY_BOARD) {
        randomMoveIndex = generateBoundedRandomNumber2(rng, board->state.totalAmountOfOpenSquares);
        currentBoard = 0;
        while (currentBoard < 9 && randomMoveIndex - board->state.amountOfOpenSquaresBySmallBoard[currentBoard] >= 0) {
            randomMoveIndex -= board->state.amountOfOpenSquaresBySmallBoard[currentBoard++];
        }
    } else {
        randomMoveIndex = generateBoundedRandomNumber2(rng, board->state.amountOfOpenSquaresBySmallBoard[currentBoard]);
    }
    uint16_t bitBoard = ~(board->state.player1.smallBoards[currentBoard] | board->state.player2.smallBoards[currentBoard]) & 511;
    return openSquares2[bitBoard][currentBoard][randomMoveIndex];
}


void makeRandomTemporaryMove2(Board2* board, RNG2* rng) {
    Player player = getCurrentPlayer2(board);
    uint16_t smallBoardsWithWinningMove = board->state.instantWinBoards[player] & board->state.instantWinSmallBoards[player];
    if (hasWinningMove2(board, smallBoardsWithWinningMove)) {
        board->state.winner = board->state.currentPlayer + 1;
        return;
    }
    Square2 move = generateMove2(board, rng);
    makeTemporaryMove2(board, move);
}


Winner rollout2(Board2* board, RNG2* rng) {
    while (getWinner2(board) == NONE) {
        makeRandomTemporaryMove2(board, rng);
    }
    return getWinner2(board);
}
// END ROLLOUT















// START BOARD
int8_t setOpenSquares2(Square2 openSquares_[9], uint8_t boardIndex, uint16_t bitBoard) {
    int8_t amountOfMoves = 0;
    while (bitBoard) {
        Square2 square = {boardIndex, __builtin_ffs(bitBoard) - 1};
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
    initializePlayerBitBoard2(&board->state.player1);
    initializePlayerBitBoard2(&board->state.player2);
    board->state.instantWinBoards[PLAYER1] = 0;
    board->state.instantWinBoards[PLAYER2] = 0;
    board->state.instantWinSmallBoards[PLAYER1] = 0;
    board->state.instantWinSmallBoards[PLAYER2] = 0;
    board->state.currentPlayer = PLAYER1;
    board->state.currentBoard = ANY_BOARD;
    board->state.winner = NONE;
    board->state.ply = 0;
    board->state.totalAmountOfOpenSquares = 81;
    for (int boardIndex = 0; boardIndex < 9; boardIndex++) {
        board->state.amountOfOpenSquaresBySmallBoard[boardIndex] = 9;
    }
    board->stateCheckpoint = board->state;
    for (int boardIndex = 0; boardIndex < 9; boardIndex++) {
        for (int bitBoard = 0; bitBoard < 512; bitBoard++) {
            amountOfOpenSquares2[bitBoard] =
                    setOpenSquares2(openSquares2[bitBoard][boardIndex], boardIndex, bitBoard);
        }
    }
    for (uint16_t player1BigBoard = 0; player1BigBoard < 512; player1BigBoard++) {
        for (uint16_t player2BigBoard = 0; player2BigBoard < 512; player2BigBoard++) {
            Winner winner = calculateWinner2(player1BigBoard, player2BigBoard);
            winnerByBigBoards2[player1BigBoard][player2BigBoard] = winner;
        }
    }
    board->me = PLAYER2;
    initializeLookupTable2();
    return board;
}


void freeBoard2(Board2* board) {
    free(board);
}


Square2* getMovesSingleBoard2(Board2* board, uint8_t boardIndex, int8_t* amountOfMoves) {
    uint16_t bitBoard = ~(board->state.player1.smallBoards[boardIndex] | board->state.player2.smallBoards[boardIndex]) & 511;
    *amountOfMoves = amountOfOpenSquares2[bitBoard];
    return openSquares2[bitBoard][boardIndex];
}


int8_t copyMovesSingleBoard2(Board2* board, uint8_t boardIndex, Square2 moves[TOTAL_SMALL_SQUARES], int8_t amountOfMoves) {
    uint16_t bitBoard = ~(board->state.player1.smallBoards[boardIndex] | board->state.player2.smallBoards[boardIndex]) & 511;
    memcpy(&moves[amountOfMoves], openSquares2[bitBoard][boardIndex],
           amountOfOpenSquares2[bitBoard] * sizeof(Square2));
    return (int8_t)(amountOfMoves + amountOfOpenSquares2[bitBoard]);
}


int8_t generateMovesAnyBoard2(Board2* board, Square2 moves[TOTAL_SMALL_SQUARES]) {
    int8_t amountOfMoves = 0;
    uint16_t undecidedSmallBoards = ~(board->state.player1.bigBoard | board->state.player2.bigBoard) & 511;
    while (undecidedSmallBoards) {
        uint8_t boardIndex = __builtin_ffs(undecidedSmallBoards) - 1;
        amountOfMoves = copyMovesSingleBoard2(board, boardIndex, moves, amountOfMoves);
        undecidedSmallBoards &= undecidedSmallBoards - 1;
    }
    return amountOfMoves;
}


Square2* generateMoves2(Board2* board, Square2 moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves) {
    if (board->state.winner != NONE) {
        *amountOfMoves = 0;
        return moves;
    }
    uint8_t currentBoard = board->state.currentBoard;
    if (currentBoard == ANY_BOARD) {
        *amountOfMoves = generateMovesAnyBoard2(board, moves);
        return moves;
    }
    return getMovesSingleBoard2(board, currentBoard, amountOfMoves);
}


uint8_t getNextBoard2(Board2* board, uint8_t previousPosition) {
    bool smallBoardIsDecided = BIT_CHECK(board->state.player1.bigBoard | board->state.player2.bigBoard, previousPosition);
    return smallBoardIsDecided ? ANY_BOARD : previousPosition;
}


bool nextBoardIsEmpty2(Board2* board) {
    uint8_t currentBoard = board->state.currentBoard;
    return currentBoard != ANY_BOARD
           && (board->state.player1.smallBoards[currentBoard] | board->state.player2.smallBoards[currentBoard]) == 0;
}


bool nextBoardHasOneMoveFromBothPlayers2(Board2* board) {
    uint8_t currentBoard = board->state.currentBoard;
    return currentBoard != ANY_BOARD
           && ONE_BIT_SET(board->state.player1.smallBoards[currentBoard])
           && ONE_BIT_SET(board->state.player2.smallBoards[currentBoard]);
}


uint8_t getCurrentBoard2(Board2* board) {
    return board->state.currentBoard;
}


Player getCurrentPlayer2(Board2* board) {
    return board->state.currentPlayer;
}


void revertToCheckpoint2(Board2* board) {
    board->state = board->stateCheckpoint;
}


void updateCheckpoint2(Board2* board) {
    board->stateCheckpoint = board->state;
}


void makeTemporaryMove2(Board2* board, Square2 square) {
    PlayerBitBoard2* p1 = &board->state.player1;
    if (setSquareOccupied2(p1 + board->state.currentPlayer, p1 + !board->state.currentPlayer, square)) {
        board->state.winner = winnerByBigBoards2[board->state.player1.bigBoard][board->state.player2.bigBoard];
        board->state.totalAmountOfOpenSquares -= board->state.amountOfOpenSquaresBySmallBoard[square.board];
        board->state.amountOfOpenSquaresBySmallBoard[square.board] = 0;
        updateBigBoardState2(board);
    } else {
        board->state.totalAmountOfOpenSquares--;
        board->state.amountOfOpenSquaresBySmallBoard[square.board]--;
    }
    updateSmallBoardState2(board, square.board);
    board->state.currentPlayer ^= 1;
    board->state.currentBoard = getNextBoard2(board, square.position);
    board->state.ply++;
}


void makePermanentMove2(Board2* board, Square2 square) {
    makeTemporaryMove2(board, square);
    updateCheckpoint2(board);
}


Winner getWinnerAfterMove2(Board2* board, Square2 square) {
    State2 tempCheckpoint = board->state;
    makeTemporaryMove2(board, square);
    Winner winner = getWinner2(board);
    board->state = tempCheckpoint;
    return winner;
}


Winner getWinner2(Board2* board) {
    return board->state.winner;
}


void setMe2(Board2* board, Player player) {
    board->me = player;
}


bool currentPlayerIsMe2(Board2* board) {
    return board->state.currentPlayer == board->me;
}


uint8_t getPly2(Board2* board) {
    return board->state.ply;
}
// END BOARD















// START MCTS_NODE
MCTSNode2* createMCTSRootNode2() {
    MCTSNode2* root = calloc(1, sizeof(MCTSNode2));
    root->amountOfChildren = -1;
    root->square.board = 9;
    root->square.position = 9;
    root->amountOfUntriedMoves = -1;
    initializePriorsLookupTable2();
    return root;
}


void initializeMCTSNode2(MCTSNode2* parent, Square2 square, MCTSNode2* node) {
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
    for (int i = 0; i < copy->amountOfChildren + copy->amountOfUntriedMoves; i++) {
        copy->children[i].parent = copy;
    }
    return copy;
}


void freeMCTSTreeRecursive2(MCTSNode2* node) {
    for (int i = 0; i < node->amountOfChildren; i++) {
        freeMCTSTreeRecursive2(&node->children[i]);
    }
    free(node->children);
}


void freeMCTSTree2(MCTSNode2* root) {
    freeMCTSTreeRecursive2(root);
    if (root->parent == NULL) {
        free(root);
    }
}


void singleChild2(MCTSNode2* node, Square2 square) {
    node->amountOfUntriedMoves = 1;
    node->children = malloc(sizeof(MCTSNode2));
    initializeMCTSNode2(node, square, &node->children[0]);
}


bool handleSpecialCases2(MCTSNode2* node, Board2* board) {
    if (nextBoardIsEmpty2(board) && getPly2(board) <= 20) {
        uint8_t currentBoard = getCurrentBoard2(board);
        Square2 sameBoard = {currentBoard, currentBoard};
        singleChild2(node, sameBoard);
        return true;
    }
    if (currentPlayerIsMe2(board) && getPly2(board) == 0) {
        Square2 bestFirstMove = {4, 4};
        singleChild2(node, bestFirstMove);
        return true;
    }
    return false;
}


void discoverChildNodes2(MCTSNode2* node, Board2* board, RNG2* rng) {
    if (node->amountOfChildren == -1) {
        node->amountOfChildren = 0;
        if (!handleSpecialCases2(node, board)) {
            Square2 movesArray[TOTAL_SMALL_SQUARES];
            int8_t amountOfMoves;
            Square2* moves = generateMoves2(board, movesArray, &amountOfMoves);
            if (getPly2(board) > 30) {
                Player player = getCurrentPlayer2(board);
                for (int i = 0; i < amountOfMoves; i++) {
                    if (getWinnerAfterMove2(board, moves[i]) == player + 1) {
                        singleChild2(node, moves[i]);
                        return;
                    }
                }
            }
            int range[amountOfMoves];
            for (int i = 0; i < amountOfMoves; i++) {
                range[i] = i;
            }
            shuffle2(range, amountOfMoves, rng);
            node->amountOfUntriedMoves = amountOfMoves;
            node->children = malloc(amountOfMoves * sizeof(MCTSNode2));
            for (int i = 0; i < amountOfMoves; i++) {
                Square2 move = moves[range[i]];
                MCTSNode2* child = &node->children[i];
                initializeMCTSNode2(node, move, child);
            }
            applyPriors2(board, node);
        }
    }
}


bool isLeafNode2(MCTSNode2* node, Board2* board, RNG2* rng) {
    if (node->sims > 0) {
        discoverChildNodes2(node, board, rng);
    }
    return node->sims == 0;
}


float fastSquareRoot2(float x) {
    __m128 in = _mm_set_ss(x);
    return _mm_cvtss_f32(_mm_mul_ss(in, _mm_rsqrt_ss(in)));
}


#define EXPLORATION_PARAMETER 0.41f
float getUCTValue2(MCTSNode2* node, float parentLogSims) {
    float exploitation = node->wins*node->simsInverted;
    float exploration = fastSquareRoot2(parentLogSims * node->simsInverted);
    return exploitation + exploration;
}


// From: https://github.com/etheory/fastapprox/blob/master/fastapprox/src/fastlog.h
float fastLog22(float x) {
    union { float f; uint32_t i; } vx = { x };
    float y = (float)vx.i;
    y *= 1.1920928955078125e-7f;
    return y - 126.94269504f;
}


#define FIRST_PLAY_URGENCY 1.10f
MCTSNode2* selectNextChild2(MCTSNode2* node) {
    float logSims = EXPLORATION_PARAMETER * EXPLORATION_PARAMETER * fastLog22(node->sims);
    MCTSNode2* highestUCTChild = NULL;
    float highestUCT = -1.0f;
    for (int i = 0; i < node->amountOfChildren; i++) {
        MCTSNode2* child = &node->children[i];
        float UCT = getUCTValue2(child, logSims);
        if (UCT > highestUCT) {
            highestUCTChild = child;
            highestUCT = UCT;
        }
    }
    if (node->amountOfUntriedMoves > 0 && highestUCT < FIRST_PLAY_URGENCY) {
        highestUCTChild = expandNextChild2(node);
    }
    return highestUCTChild;
}


MCTSNode2* expandNextChild2(MCTSNode2* node) {
    node->amountOfUntriedMoves--;
    return &node->children[node->amountOfChildren++];
}


MCTSNode2* updateRoot2(MCTSNode2* root, Board2* board, Square2 square) {
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
        Square2 movesArray[TOTAL_SMALL_SQUARES];
        int8_t amountOfMoves;
        Square2* moves = generateMoves2(board, movesArray, &amountOfMoves);
        for (int i = 0; i < amountOfMoves; i++) {
            if (squaresAreEqual2(moves[i], square)) {
                MCTSNode2 temp;
                initializeMCTSNode2(NULL, square, &temp);
                newRoot = copyMCTSNode2(&temp);
                break;
            }
        }
    } else {
        newRoot->parent = NULL;
        newRoot = copyMCTSNode2(newRoot);
    }
    free(root->children);
    free(root);
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


Square2 getMostPromisingMove2(MCTSNode2* node) {
    MCTSNode2* highestWinrateChild = &node->children[0];
    float highestWinrate = getWinrate2(highestWinrateChild);
    for (int i = 1; i < node->amountOfChildren; i++) {
        MCTSNode2* child = &node->children[i];
        float winrate = getWinrate2(child);
        if (winrate > highestWinrate) {
            highestWinrateChild = child;
            highestWinrate = winrate;
        }
    }
    return highestWinrateChild->square;
}


float getWinrate2(MCTSNode2* node) {
    return node->wins * node->simsInverted;
}
// END MCTS_NODE
















// START PRIORS
bool pairPriors2[512][512][9];
void calculatePairPriors2(uint16_t smallBoard, uint16_t otherPlayerSmallBoard, bool* result) {
    uint16_t linesOf2Masks[24] = {
            3, 6, 24, 48, 192, 384, 5, 40, 320,  // horizontal
            9, 18, 36, 72, 144, 288, 65, 130, 260,  // vertical
            17, 272, 20, 80, 257, 68  // diagonal
    };
    for (uint8_t position = 0; position < 9; position++) {
        result[position] = false;
        if ((1 << position) == smallBoard || (1 << position) == otherPlayerSmallBoard) {
            continue;
        }
        for (int i = 0; i < 24; i++) {
            uint16_t mask = linesOf2Masks[i];
            if ((smallBoard | (1 << position)) == mask && !isWin2(mask | otherPlayerSmallBoard)) {
                result[position] = true;
                break;
            }
        }
    }
}


void initializePriorsLookupTable2() {
    for (uint8_t position1 = 0; position1 < 9; position1++) {
        for (uint8_t position2 = 0; position2 < 9; position2++) {
            if (position1 != position2) {
                uint16_t smallBoard = 1 << position1;
                uint16_t otherPlayerSmallBoard = 1 << position2;
                calculatePairPriors2(smallBoard, otherPlayerSmallBoard, pairPriors2[smallBoard][otherPlayerSmallBoard]);
            }
        }
    }
}


void applyPairPriors2(MCTSNode2* parent, uint16_t smallBoard, uint16_t otherPlayerSmallBoard, float prior) {
    bool* formsPair = pairPriors2[smallBoard][otherPlayerSmallBoard];
    for (int i = 0; i < parent->amountOfUntriedMoves; i++) {
        MCTSNode2* child = &parent->children[i];
        child->wins = formsPair[child->square.position]? prior : 0.0f;
        child->sims = prior;
        child->simsInverted = 1.0f / prior;
    }
}


void applySendToDecidedBoardPriors2(Board2* board, MCTSNode2* parent, float prior) {
    for (int i = 0; i < parent->amountOfUntriedMoves; i++) {
        MCTSNode2* child = &parent->children[i];
        if ((board->state.player1.bigBoard | board->state.player2.bigBoard) & (1 << child->square.position)) {
            child->wins = 0.0f;
            child->sims = prior;
            child->simsInverted = 1.0f / prior;
        }
    }
}


void applyPriors2(Board2* board, MCTSNode2* parent) {
    PlayerBitBoard2* p1 = &board->state.player1;
    uint16_t smallBoard = (p1 + board->state.currentPlayer)->smallBoards[board->state.currentBoard];
    uint16_t otherPlayerSmallBoard = (p1 + !board->state.currentPlayer)->smallBoards[board->state.currentBoard];
    uint8_t ply = getPly2(board);
    if (nextBoardHasOneMoveFromBothPlayers2(board) && ply <= 30) {
        applyPairPriors2(parent, smallBoard, otherPlayerSmallBoard, ply <= 18 ? 1000.0f : 2.0f);
    }
    if (ply <= 40) {
        applySendToDecidedBoardPriors2(board, parent, ply <= 30 ? 1000.0f : 5.0f);
    }
}
// END PRIORS


















// START SOLVER
#define BIG_FLOAT 5000000.0f


void checkAllSet2(MCTSNode2* node) {
    if (node == NULL || node->amountOfUntriedMoves > 0) {
        return;
    }
    bool hasDraw = false;
    for (int i = 0; i < node->amountOfChildren; i++) {
        MCTSNode2 child = node->children[i];
        if (child.sims < BIG_FLOAT || child.wins == child.sims) {
            return;
        }
        if (!hasDraw && child.wins > 0) {
            hasDraw = true;
        }
    }
    setNodeWinner2(node, hasDraw ? DRAW : WIN_P1, PLAYER1);
}


void setNodeWinner2(MCTSNode2* node, Winner winner, Player player) {
    if (node == NULL || node->sims >= BIG_FLOAT) {
        return;
    }
    node->sims = BIG_FLOAT;
    node->simsInverted = 1.0f / BIG_FLOAT;
    if (winner == DRAW) {
        node->wins = BIG_FLOAT / 2.0f;
        checkAllSet2(node->parent);
    } else if (player + 1 == winner) {
        node->wins = BIG_FLOAT;
        setNodeWinner2(node->parent, winner, OTHER_PLAYER(player));
    } else {
        node->wins = 0;
        checkAllSet2(node->parent);
    }
}
// END SOLVER


















// START FIND_NEXT_MOVE
MCTSNode2* selectLeaf2(Board2* board, MCTSNode2* root, RNG2* rng) {
    MCTSNode2* currentNode = root;
    while (!isLeafNode2(currentNode, board, rng) && getWinner2(board) == NONE) {
        currentNode = selectNextChild2(currentNode);
        visitNode2(currentNode, board);
    }
    return currentNode;
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
        MCTSNode2* leaf = selectLeaf2(board, root, rng);
        Winner winner = getWinner2(board);
        Player player = OTHER_PLAYER(getCurrentPlayer2(board));
        if (winner == NONE) {
            winner = rollout2(board, rng);
        } else {
            setNodeWinner2(leaf, winner, player);
        }
        backpropagate2(leaf, winner, player);
        revertToCheckpoint2(board);
    }
    return amountOfSimulations;
}
// END FIND_NEXT_MOVE

















// START HANDLE_TURN
Square2 handleOpening2(Board2* board) {
    uint8_t ply = getPly2(board);
    Square2 invalid = {9, 9};
    if (board->me != PLAYER2 || ply > 10) {
        return invalid;
    }
    bool firstMove = BIT_CHECK(board->state.player1.smallBoards[4], 4);
    bool secondMove = BIT_CHECK(board->state.player1.smallBoards[8], 8);
    bool thirdMove = BIT_CHECK(board->state.player1.smallBoards[0], 0);
    bool fourthMove = BIT_CHECK(board->state.player1.smallBoards[5], 5);
    bool fifthMove = BIT_CHECK(board->state.player1.smallBoards[7], 7);
    uint8_t currentBoard = getCurrentBoard2(board);
    if (ply == 1 && firstMove) {
        Square2 move = {4, 8};
        return move;
    } else if (ply == 3 && firstMove && secondMove && currentBoard == 8) {
        Square2 move = {8, 0};
        return move;
    } else if (ply == 5 && firstMove && secondMove && thirdMove && currentBoard == 0) {
        Square2 move = {0, 5};
        return move;
    } else if (ply == 7 && firstMove && secondMove && thirdMove && fourthMove && currentBoard == 5) {
        Square2 move = {5, 7};
        return move;
    } else if (ply == 9 && firstMove && secondMove && thirdMove && fourthMove && fifthMove && currentBoard == 7) {
        Square2 move = {7, 5};
        return move;
    }
    return invalid;
}


MCTSNode2* handleEnemyTurn2(Board2* board, MCTSNode2* root, Square2 enemyMove, RNG2* rng) {
    if (enemyMove.board == 9 && enemyMove.position == 9) {
        setMe2(board, PLAYER1);
        return root;
    }
    discoverChildNodes2(root, board, rng);
    MCTSNode2* newRoot = updateRoot2(root, board, enemyMove);
    makePermanentMove2(board, enemyMove);
    return newRoot;
}


HandleTurnResult2 handleTurn2(Board2* board, MCTSNode2* root, RNG2* rng, double allocatedTime, Square2 enemyMove) {
    root = handleEnemyTurn2(board, root, enemyMove, rng);
    Square2 openingMove = {9, 9};
    if (openingMove.board != 9) {
        MCTSNode2* newRoot = updateRoot2(root, board, openingMove);
        makePermanentMove2(board, openingMove);
        HandleTurnResult2 result = {openingMove, newRoot, 0};
        return result;
    }
    int amountOfSimulations = findNextMove2(board, root, rng, allocatedTime/100);
    Square2 move = getMostPromisingMove2(root);
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


float getWinrateOpponent(StateOpponent* state) {
    return getWinrate2(state->root);
}


Square2 playTurnOpponent(StateOpponent** state, double allocatedTime, Square2 enemyMove) {
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