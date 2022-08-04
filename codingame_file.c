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

typedef struct Square {
    uint8_t board;
    uint8_t position;
} Square;

Square createSquare(uint8_t board, uint8_t position);

bool squaresAreEqual(Square square1, Square square2);

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CHECK(a,b) ((a) & (1ULL<<(b)))
#define BIT_CHANGE(a,b,c) ((a) = (a) & ~(b) | (-(c) & (b)))

void crash(char* errorMessage);

Square toOurNotation(Square rowAndColumn);

Square toGameNotation(Square square);

typedef struct RNG {
    uint64_t state;
    uint64_t inc;
} RNG;

void seedRNG(RNG* rng, uint64_t init_state, uint64_t init_seq);

uint8_t generateBoundedRandomNumber(RNG* rng, uint8_t bound);

#define Player bool
#define PLAYER1 0
#define PLAYER2 1
#define OTHER_PLAYER(p) ((p)^1)

#define Winner uint8_t
#define NONE 0
#define WIN_P1 1
#define WIN_P2 2
#define DRAW 3


typedef struct PlayerBitBoard {
    uint16_t smallBoards[9];
    uint16_t bigBoard;
    uint16_t checkpointSmallBoards[9];
    uint16_t checkpointBigBoard;
} PlayerBitBoard;

PlayerBitBoard* createPlayerBitBoard();

void initializePlayerBitBoard(PlayerBitBoard* playerBitBoard);

void freePlayerBitBoard(PlayerBitBoard* playerBitBoard);

bool boardIsWon(PlayerBitBoard* playerBitBoard, uint8_t board);

bool squareIsOccupied(PlayerBitBoard* playerBitBoard, Square square);

bool isWin(uint16_t smallBoard);

bool setSquareOccupied(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayerBitBoard, Square square);

void revertToPlayerCheckpoint(PlayerBitBoard* playerBitBoard);

void updatePlayerCheckpoint(PlayerBitBoard* playerBitBoard);

#define TOTAL_SMALL_SQUARES 81
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
    PlayerBitBoard player1;
    PlayerBitBoard player2;
    AdditionalState AS;
    AdditionalState ASCheckpoint;
    Player me;
} Board;

Square openSquares[512][9][9];
int8_t amountOfOpenSquares[512];
Winner winnerByBigBoards[512][512];

typedef struct Board Board;

Board* createBoard();

void freeBoard(Board* board);

Square* generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves);

uint8_t getNextBoard(Board* board, uint8_t previousPosition);

bool nextBoardIsEmpty(Board* board);

uint8_t getCurrentBoard(Board* board);

Player getCurrentPlayer(Board* board);

void revertToCheckpoint(Board* board);

void makeTemporaryMove(Board* board, Square square);

void makePermanentMove(Board* board, Square square);

Winner getWinner(Board* board);

void setMe(Board* board, Player player);

bool currentPlayerIsMe(Board* board);

uint8_t getPly(Board* board);

typedef struct RolloutState {
    uint16_t instantWinBoards[2];
    uint16_t instantWinSmallBoards[2];
} RolloutState;

void initializeLookupTable();

void initializeRolloutState(RolloutState* RS);

bool hasWinningMove(Board* board, RolloutState* RS);

void updateSmallBoardState(Board* board, RolloutState* RS, uint8_t boardIndex);

void updateBigBoardState(Board* board, RolloutState* RS);

void makeRandomTemporaryMove(Board* board, RolloutState* RS, RNG* rng);

Winner rollout(Board* board, RNG* rng);

typedef struct MCTSNode MCTSNode;

MCTSNode* createMCTSRootNode();

void freeMCTSTree(MCTSNode* root);

bool isLeafNode(MCTSNode* node, Board* board);

MCTSNode* selectNextChild(MCTSNode* node);

MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square);

void backpropagate(MCTSNode* node, Winner winner, Player player);

void visitNode(MCTSNode* node, Board* board);

void setNodeWinner(MCTSNode* node, Winner winner, Player player);

Square getMostPromisingMove(MCTSNode* node);

int getSims(MCTSNode* node);

float getWinrate(MCTSNode* node);

int findNextMove(Board* board, MCTSNode* root, RNG* rng, double allocatedTime);

typedef struct HandleTurnResult {
    Square move;
    MCTSNode* newRoot;
    int amountOfSimulations;
} HandleTurnResult;

HandleTurnResult handleTurn(Board* board, MCTSNode* root, RNG* rng, double allocatedTime, Square enemyMove);

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
uint32_t pcg32_random_r(RNG* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}


void seedRNG(RNG* rng, uint64_t init_state, uint64_t init_seq) {
    rng->state = 0U;
    rng->inc = (init_seq << 1u) | 1u;
    pcg32_random_r(rng);
    rng->state += init_state;
    pcg32_random_r(rng);
}


uint8_t generateBoundedRandomNumber(RNG* rng, uint8_t bound) {
    return ((uint64_t)pcg32_random_r(rng) * (uint64_t)bound) >> 32;
}
// END RANDOM









// START PLAYER_BIT_BOARD
bool precalculatedIsWin[512];
bool isWin_(uint16_t smallBoard) {
    __m128i masks = _mm_setr_epi16(0x7, 0x38, 0x1c0, 0x49, 0x92, 0x124, 0x111, 0x54);
    __m128i v1 = _mm_set1_epi16(1);
    __m128i boards128 = _mm_set1_epi16((short) smallBoard);
    __m128i andResult = _mm_and_si128(masks, boards128);
    __m128i result = _mm_cmpeq_epi16(andResult, masks);
    return !_mm_test_all_zeros(result, v1);
}


bool isWin(uint16_t smallBoard) {
    return precalculatedIsWin[smallBoard];
}


PlayerBitBoard* createPlayerBitBoard() {
    for (uint16_t i = 0; i < 512; i++) {
        precalculatedIsWin[i] = isWin_(i);
    }
    return calloc(1, sizeof(PlayerBitBoard));
}


void initializePlayerBitBoard(PlayerBitBoard* playerBitBoard) {
    for (uint16_t i = 0; i < 512; i++) {
        precalculatedIsWin[i] = isWin_(i);
    }
    memset(playerBitBoard, 0, sizeof(PlayerBitBoard));
}


void freePlayerBitBoard(PlayerBitBoard* playerBitBoard) {
    free(playerBitBoard);
}


bool boardIsWon(PlayerBitBoard* playerBitBoard, uint8_t board) {
    return BIT_CHECK(playerBitBoard->bigBoard, board);
}


bool squareIsOccupied(PlayerBitBoard* playerBitBoard, Square square) {
    return BIT_CHECK(playerBitBoard->smallBoards[square.board], square.position);
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
    memcpy(playerBitBoard->smallBoards, playerBitBoard->checkpointSmallBoards, 9 * sizeof(uint16_t));
    playerBitBoard->bigBoard = playerBitBoard->checkpointBigBoard;
}


void updatePlayerCheckpoint(PlayerBitBoard* playerBitBoard) {
    memcpy(playerBitBoard->checkpointSmallBoards, playerBitBoard->smallBoards, 9 * sizeof(uint16_t));
    playerBitBoard->checkpointBigBoard = playerBitBoard->bigBoard;
}
// END PLAYER_BIT_BOARD









// START SMART_ROLLOUT
uint16_t precalculatedInstantWinBoards[512];
uint16_t calculateInstantWinBoards_(uint16_t bigBoard) {
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


uint16_t calculateInstantWinBoards(uint16_t bigBoard, uint16_t otherPlayerBigBoard) {
    return precalculatedInstantWinBoards[bigBoard & ~otherPlayerBigBoard] & ~otherPlayerBigBoard;
}


bool smallBoardHasInstantWinMove(uint16_t smallBoard, uint16_t otherSmallBoard) {
    return calculateInstantWinBoards(smallBoard, otherSmallBoard) != 0;
}


void initializeLookupTable() {
    for (uint16_t bigBoard = 0; bigBoard < 512; bigBoard++) {
        precalculatedInstantWinBoards[bigBoard] = calculateInstantWinBoards_(bigBoard);
    }
}


void initializeRolloutState(RolloutState* RS) {
    memset(RS, 0, sizeof(RolloutState));
}


bool canWinWith3InARow(uint8_t currentBoard, uint16_t smallBoardsWithWinningMove) {
    return (currentBoard == ANY_BOARD && smallBoardsWithWinningMove)
           || BIT_CHECK(smallBoardsWithWinningMove, currentBoard);
}


bool hasWinningMove(Board* board, RolloutState* RS) {
    Player player = board->AS.currentPlayer;
    uint8_t currentBoard = board->AS.currentBoard;
    uint16_t smallBoardsWithWinningMove = RS->instantWinBoards[player] & RS->instantWinSmallBoards[player];
    return canWinWith3InARow(currentBoard, smallBoardsWithWinningMove);
}


void updateSmallBoardState(Board* board, RolloutState* RS, uint8_t boardIndex) {
    uint16_t p1 = board->player1.smallBoards[boardIndex];
    uint16_t p2 = board->player2.smallBoards[boardIndex];
    uint16_t mask = 1ULL << boardIndex;
    BIT_CHANGE(RS->instantWinSmallBoards[PLAYER1], mask, smallBoardHasInstantWinMove(p1, p2));
    BIT_CHANGE(RS->instantWinSmallBoards[PLAYER2], mask, smallBoardHasInstantWinMove(p2, p1));
}


void updateBigBoardState(Board* board, RolloutState* RS) {
    uint16_t p1 = board->player1.bigBoard;
    uint16_t p2 = board->player2.bigBoard;
    RS->instantWinBoards[PLAYER1] = calculateInstantWinBoards(p1, p2);
    RS->instantWinBoards[PLAYER2] = calculateInstantWinBoards(p2, p1);
}
// END SMART_ROLLOUT










// START ROLLOUT
void makeRolloutTemporaryMove(Board* board, RolloutState* RS, Square square) {
    PlayerBitBoard* p1 = &board->player1;
    if (setSquareOccupied(p1 + board->AS.currentPlayer, p1 + !board->AS.currentPlayer, square)) {
        board->AS.winner = winnerByBigBoards[board->player1.bigBoard][board->player2.bigBoard];
        board->AS.totalAmountOfOpenSquares -= board->AS.amountOfOpenSquaresBySmallBoard[square.board];
        board->AS.amountOfOpenSquaresBySmallBoard[square.board] = 0;
        updateBigBoardState(board, RS);
    } else {
        board->AS.totalAmountOfOpenSquares--;
        board->AS.amountOfOpenSquaresBySmallBoard[square.board]--;
    }
    updateSmallBoardState(board, RS, square.board);
    board->AS.currentPlayer ^= 1;
    board->AS.currentBoard = getNextBoard(board, square.position);
    board->AS.ply++;
}


void makeRandomTemporaryMove(Board* board, RolloutState* RS, RNG* rng) {
    uint8_t currentBoard = board->AS.currentBoard;
    if (hasWinningMove(board, RS)) {
        board->AS.winner = board->AS.currentPlayer + 1;
        return;
    }
    uint8_t randomMoveIndex;
    if (currentBoard == ANY_BOARD) {
        randomMoveIndex = generateBoundedRandomNumber(rng, board->AS.totalAmountOfOpenSquares);
        currentBoard = 0;
        while (currentBoard < 9 && randomMoveIndex < 128) {
            randomMoveIndex -= board->AS.amountOfOpenSquaresBySmallBoard[currentBoard++];
        }
        randomMoveIndex += board->AS.amountOfOpenSquaresBySmallBoard[--currentBoard];
    } else {
        randomMoveIndex = generateBoundedRandomNumber(rng, board->AS.amountOfOpenSquaresBySmallBoard[currentBoard]);
    }
    uint16_t bitBoard = ~(board->player1.smallBoards[currentBoard] | board->player2.smallBoards[currentBoard]) & 511;
    makeRolloutTemporaryMove(board, RS, openSquares[bitBoard][currentBoard][randomMoveIndex]);
}


Winner rollout(Board* board, RNG* rng) {
    RolloutState state;
    initializeRolloutState(&state);
    while (getWinner(board) == NONE) {
        makeRandomTemporaryMove(board, &state, rng);
    }
    return getWinner(board);
}
// END ROLLOUT















// START BOARD
int8_t setOpenSquares(Square openSquares_[9], uint8_t boardIndex, uint16_t bitBoard) {
    int8_t amountOfMoves = 0;
    while (bitBoard) {
        Square square = {boardIndex, __builtin_ffs(bitBoard) - 1};
        openSquares_[amountOfMoves++] = square;
        bitBoard &= bitBoard - 1;
    }
    return amountOfMoves;
}


Winner calculateWinner(uint16_t player1BigBoard, uint16_t player2BigBoard) {
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


Board* createBoard() {
    Board* board = malloc(sizeof(Board));
    initializePlayerBitBoard(&board->player1);
    initializePlayerBitBoard(&board->player2);
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
                    setOpenSquares(openSquares[bitBoard][boardIndex], boardIndex, bitBoard);
        }
    }
    for (uint16_t player1BigBoard = 0; player1BigBoard < 512; player1BigBoard++) {
        for (uint16_t player2BigBoard = 0; player2BigBoard < 512; player2BigBoard++) {
            Winner winner = calculateWinner(player1BigBoard, player2BigBoard);
            winnerByBigBoards[player1BigBoard][player2BigBoard] = winner;
        }
    }
    board->me = PLAYER2;
    initializeLookupTable();
    return board;
}


void freeBoard(Board* board) {
    free(board);
}


Square* getMovesSingleBoard(Board* board, uint8_t boardIndex, int8_t* amountOfMoves) {
    uint16_t bitBoard = ~(board->player1.smallBoards[boardIndex] | board->player2.smallBoards[boardIndex]) & 511;
    *amountOfMoves = amountOfOpenSquares[bitBoard];
    return openSquares[bitBoard][boardIndex];
}


int8_t copyMovesSingleBoard(Board* board, uint8_t boardIndex, Square moves[TOTAL_SMALL_SQUARES], int8_t amountOfMoves) {
    uint16_t bitBoard = ~(board->player1.smallBoards[boardIndex] | board->player2.smallBoards[boardIndex]) & 511;
    memcpy(&moves[amountOfMoves], openSquares[bitBoard][boardIndex],
           amountOfOpenSquares[bitBoard] * sizeof(Square));
    return (int8_t)(amountOfMoves + amountOfOpenSquares[bitBoard]);
}


int8_t generateMovesAnyBoard(Board* board, Square moves[TOTAL_SMALL_SQUARES]) {
    int8_t amountOfMoves = 0;
    uint16_t undecidedSmallBoards = ~(board->player1.bigBoard | board->player2.bigBoard) & 511;
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


uint8_t getNextBoard(Board* board, uint8_t previousPosition) {
    bool smallBoardIsDecided = BIT_CHECK(board->player1.bigBoard | board->player2.bigBoard, previousPosition);
    return smallBoardIsDecided ? ANY_BOARD : previousPosition;
}


bool nextBoardIsEmpty(Board* board) {
    uint8_t currentBoard = board->AS.currentBoard;
    return currentBoard != ANY_BOARD
           && (board->player1.smallBoards[currentBoard] | board->player2.smallBoards[currentBoard]) == 0;
}


uint8_t getCurrentBoard(Board* board) {
    return board->AS.currentBoard;
}


Player getCurrentPlayer(Board* board) {
    return board->AS.currentPlayer;
}


void revertToCheckpoint(Board* board) {
    revertToPlayerCheckpoint(&board->player1);
    revertToPlayerCheckpoint(&board->player2);
    board->AS = board->ASCheckpoint;
}


void updateCheckpoint(Board* board) {
    updatePlayerCheckpoint(&board->player1);
    updatePlayerCheckpoint(&board->player2);
    board->ASCheckpoint = board->AS;
}


void makeTemporaryMove(Board* board, Square square) {
    PlayerBitBoard* p1 = &board->player1;
    if (setSquareOccupied(p1 + board->AS.currentPlayer, p1 + !board->AS.currentPlayer, square)) {
        board->AS.winner = winnerByBigBoards[board->player1.bigBoard][board->player2.bigBoard];
        board->AS.totalAmountOfOpenSquares -= board->AS.amountOfOpenSquaresBySmallBoard[square.board];
        board->AS.amountOfOpenSquaresBySmallBoard[square.board] = 0;
    } else {
        board->AS.totalAmountOfOpenSquares--;
        board->AS.amountOfOpenSquaresBySmallBoard[square.board]--;
    }
    board->AS.currentPlayer ^= 1;
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
    MCTSNode* children;
    float wins;
    float sims;
    float simsInverted;
    Square square;
    int8_t amountOfChildren;
    int8_t amountOfUntriedMoves;
} MCTSNode;


MCTSNode* createMCTSRootNode() {
    MCTSNode* root = calloc(1, sizeof(MCTSNode));
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
    node->simsInverted = 0.0f;
    node->square = square;
    node->amountOfChildren = -1;
    node->amountOfUntriedMoves = -1;
}


MCTSNode* copyMCTSNode(MCTSNode* original) {
    MCTSNode* copy = malloc(sizeof(MCTSNode));
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


void freeMCTSTree(MCTSNode* node) {
    for (int i = 0; i < node->amountOfChildren; i++) {
        freeMCTSTree(&node->children[i]);
    }
    free(node->children);
    if (node->parent == NULL) {
        free(node);
    }
}


void singleChild(MCTSNode* node, Square square) {
    node->amountOfUntriedMoves = 1;
    node->children = malloc(sizeof(MCTSNode));
    node->children[0].square = square;
}


bool handleSpecialCases(MCTSNode* node, Board* board) {
    if (nextBoardIsEmpty(board) && currentPlayerIsMe(board) && getPly(board) <= 20) {
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


void discoverChildNodes(MCTSNode* node, Board* board) {
    if (node->amountOfChildren == -1) {
        node->amountOfChildren = 0;
        if (!handleSpecialCases(node, board)) {
            Square movesArray[TOTAL_SMALL_SQUARES];
            int8_t amountOfMoves;
            Square* moves = generateMoves(board, movesArray, &amountOfMoves);
            node->amountOfUntriedMoves = amountOfMoves;
            node->children = malloc(amountOfMoves * sizeof(MCTSNode));
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


void fastSquareRoot(float* restrict pOut, float* restrict pIn) {
    __m128 in = _mm_load_ss(pIn);
    _mm_store_ss(pOut, _mm_mul_ss(in, _mm_rsqrt_ss(in)));
}


#define EXPLORATION_PARAMETER 0.459375f
float getUCTValue(MCTSNode* node, float parentLogSims) {
    float c = EXPLORATION_PARAMETER;
    float sqrtIn = parentLogSims * node->simsInverted;
    float sqrtOut;
    fastSquareRoot(&sqrtOut, &sqrtIn);
    return node->wins*node->simsInverted + c*sqrtOut;
}


MCTSNode* expandNode(MCTSNode* node, int childIndex) {
    MCTSNode* newChild = &node->children[childIndex + node->amountOfChildren++];
    initializeMCTSNode(node, newChild->square, newChild);
    node->amountOfUntriedMoves--;
    return newChild;
}


// From: https://github.com/etheory/fastapprox/blob/master/fastapprox/src/fastlog.h
float fastLog2(float x) {
    union { float f; uint32_t i; } vx = { x };
    float y = (float)vx.i;
    y *= 1.1920928955078125e-7f;
    return y - 126.94269504f;
}


MCTSNode* selectNextChild(MCTSNode* node) {
    if (node->amountOfUntriedMoves) {
        return expandNode(node, 0);
    }
    float logSims = fastLog2(node->sims);
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
    return highestUCTChild;
}


MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square) {
    discoverChildNodes(root, board);
    MCTSNode* newRoot = NULL;
    for (int i = 0; i < root->amountOfChildren; i++) {
        MCTSNode* child = &root->children[i];
        if (squaresAreEqual(square, child->square)) {
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
    newRoot->parent = NULL;
    newRoot = copyMCTSNode(newRoot);
    free(root->children);
    free(root);
    return newRoot;
}


void backpropagate(MCTSNode* node, Winner winner, Player player) {
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


#define A_LOT 100000.0f
void setNodeWinner(MCTSNode* node, Winner winner, Player player) {
    if (winner != DRAW) {
        bool win = player + 1 == winner;
        node->wins += win? A_LOT : -A_LOT;
        if (!win) {
            node->parent->wins += A_LOT;
        }
    }
}


Square getMostPromisingMove(MCTSNode* node) {
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
    return node->wins * node->simsInverted;
}
// END MCTS_NODE

















// START FIND_NEXT_MOVE
MCTSNode* selectLeaf(Board* board, MCTSNode* root) {
    MCTSNode* currentNode = root;
    while (!isLeafNode(currentNode, board) && getWinner(board) == NONE) {
        currentNode = selectNextChild(currentNode);
        visitNode(currentNode, board);
    }
    return currentNode;
}


MCTSNode* expandLeaf(Board* board, MCTSNode* leaf) {
    MCTSNode* nextChild = selectNextChild(leaf);
    visitNode(nextChild, board);
    return nextChild;
}


Winner simulate(Board* board, RNG* rng) {
    RolloutState RS;
    initializeRolloutState(&RS);
    while (getWinner(board) == NONE) {
        makeRandomTemporaryMove(board, &RS, rng);
    }
    return getWinner(board);
}


bool hasTimeRemaining(struct timeval start, double allocatedTime) {
    struct timeval end;
    gettimeofday(&end, NULL);
    double timePassed = (double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec);
    return timePassed < allocatedTime;
}


int findNextMove(Board* board, MCTSNode* root, RNG* rng, double allocatedTime) {
    int amountOfSimulations = 0;
    struct timeval start;
    gettimeofday(&start, NULL);
    while (++amountOfSimulations % 128 != 0 || hasTimeRemaining(start, allocatedTime)) {
        MCTSNode* leaf = selectLeaf(board, root);
        MCTSNode* playoutNode;
        Winner simulationWinner;
        Winner winner = getWinner(board);
        Player player;
        if (winner == NONE) {
            playoutNode = expandLeaf(board, leaf);
            player = OTHER_PLAYER(getCurrentPlayer(board));
            simulationWinner = simulate(board, rng);
        } else {
            playoutNode = leaf;
            simulationWinner = winner;
            player = OTHER_PLAYER(getCurrentPlayer(board));
            setNodeWinner(playoutNode, winner, player);
        }
        backpropagate(playoutNode, simulationWinner, player);
        revertToCheckpoint(board);
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


HandleTurnResult handleTurn(Board* board, MCTSNode* root, RNG* rng, double allocatedTime, Square enemyMove) {
    root = handleEnemyTurn(board, root, enemyMove);
    int amountOfSimulations = findNextMove(board, root, rng, allocatedTime);
    Square move = getMostPromisingMove(root);
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
    float winrate = getWinrate(root);
    printf("%d %d %.4f %d\n", x, y, winrate, amountOfSimulations);
    fflush(stdout);
}


Square playTurn(Board* board, MCTSNode** root, RNG* rng, double allocatedTime, Square enemyMove) {
    HandleTurnResult result = handleTurn(board, *root, rng, allocatedTime, enemyMove);
    *root = result.newRoot;
    return result.move;
}


void playGame(FILE* file, double timePerMove) {
    Board* board = createBoard();
    MCTSNode* root = createMCTSRootNode();
    RNG rng;
    seedRNG(&rng, 69, 420);
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


#define TIME 0.095
int main() {
    // runTests();
    playGame(stdin, TIME);
}
// END MAIN