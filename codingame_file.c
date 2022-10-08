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
#define ONE_BIT_SET(a) (((a) & ((a)-1)) == 0 && (a) != 0)

Square toOurNotation(Square rowAndColumn);

Square toGameNotation(Square square);

typedef struct RNG {
    uint64_t state;
    uint64_t inc;
} RNG;

void seedRNG(RNG* rng, uint64_t init_state, uint64_t init_seq);

uint8_t generateBoundedRandomNumber(RNG* rng, uint8_t bound);

void shuffle(int* array, int n, RNG* rng);

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
} PlayerBitBoard;

void initializePlayerBitBoard(PlayerBitBoard* playerBitBoard);

bool boardIsWon(PlayerBitBoard* playerBitBoard, uint8_t board);

bool squareIsOccupied(PlayerBitBoard* playerBitBoard, Square square);

bool isWin(uint16_t smallBoard);

bool setSquareOccupied(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayerBitBoard, Square square);

#define TOTAL_SMALL_SQUARES 81
#define ANY_BOARD 9

typedef struct State {
    PlayerBitBoard player1;
    PlayerBitBoard player2;
    uint16_t instantWinBoards[2];
    uint16_t instantWinSmallBoards[2];
    Player currentPlayer;
    uint8_t currentBoard;
    Winner winner;
    uint8_t ply;
    uint8_t totalAmountOfOpenSquares;
    uint8_t amountOfOpenSquaresBySmallBoard[9];
} State;

typedef struct Board {
    State state;
    State stateCheckpoint;
    Player me;
} Board;

Square openSquares[512][9][9];
int8_t amountOfOpenSquares[512];
Winner winnerByBigBoards[512][512];

Board* createBoard();

void freeBoard(Board* board);

Square* generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves);

uint8_t getNextBoard(Board* board, uint8_t previousPosition);

bool nextBoardIsEmpty(Board* board);

bool nextBoardHasOneMoveFromBothPlayers(Board* board);

uint8_t getCurrentBoard(Board* board);

Player getCurrentPlayer(Board* board);

void revertToCheckpoint(Board* board);

void makeTemporaryMove(Board* board, Square square);

void makePermanentMove(Board* board, Square square);

Winner getWinnerAfterMove(Board* board, Square square);

Winner getWinner(Board* board);

void setMe(Board* board, Player player);

bool currentPlayerIsMe(Board* board);

uint8_t getPly(Board* board);

typedef struct RolloutState {
    uint16_t instantWinBoards[2];
    uint16_t instantWinSmallBoards[2];
} RolloutState;

void initializeLookupTable();

bool hasWinningMove(Board* board, uint16_t smallBoardsWithWinningMove);

void updateSmallBoardState(Board* board, uint8_t boardIndex);

void updateBigBoardState(Board* board);

void makeRandomTemporaryMove(Board* board, RNG* rng);

Winner rollout(Board* board, RNG* rng);

typedef struct MCTSNode {
    struct MCTSNode* parent;
    struct MCTSNode* children;
    float wins;
    float sims;
    float simsInverted;
    Square square;
    int8_t amountOfChildren;
    int8_t amountOfUntriedMoves;
} MCTSNode;

MCTSNode* createMCTSRootNode();

void freeMCTSTree(MCTSNode* root);

void discoverChildNodes(MCTSNode* node, Board* board, RNG* rng);

bool isLeafNode(MCTSNode* node, Board* board, RNG* rng);

MCTSNode* selectNextChild(MCTSNode* node);

MCTSNode* expandNextChild(MCTSNode* node);

MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square);

void backpropagate(MCTSNode* node, Winner winner, Player player);

void visitNode(MCTSNode* node, Board* board);

Square getMostPromisingMove(MCTSNode* node);

float getWinrate(MCTSNode* node);

void initializePriorsLookupTable();

bool* getPairPriors(uint16_t smallBoard, uint16_t otherPlayerSmallBoard);

void applyPriors(Board* board, MCTSNode* parent);

void setNodeWinner(MCTSNode* node, Winner winner, Player player);

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
uint32_t generateRandomNumber(RNG* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}


void seedRNG(RNG* rng, uint64_t init_state, uint64_t init_seq) {
    rng->state = 0U;
    rng->inc = (init_seq << 1u) | 1u;
    generateRandomNumber(rng);
    rng->state += init_state;
    generateRandomNumber(rng);
}


uint8_t generateBoundedRandomNumber(RNG* rng, uint8_t bound) {
    return ((uint64_t) generateRandomNumber(rng) * (uint64_t)bound) >> 32;
}


void shuffle(int* array, int n, RNG* rng) {
    uint32_t maxRandomNumber = (1L << 32) - 1;
    for (int i = 0; i < n - 1; i++) {
        uint32_t j = i + generateRandomNumber(rng) / (maxRandomNumber / (n - i) + 1);
        int t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}
// END RANDOM









// START PLAYER_BIT_BOARD
bool isWin(uint16_t smallBoard) {
    __m128i masks = _mm_setr_epi16(0x7, 0x38, 0x1c0, 0x49, 0x92, 0x124, 0x111, 0x54);
    __m128i v1 = _mm_set1_epi16(1);
    __m128i boards128 = _mm_set1_epi16((short) smallBoard);
    __m128i andResult = _mm_and_si128(masks, boards128);
    __m128i result = _mm_cmpeq_epi16(andResult, masks);
    return !_mm_test_all_zeros(result, v1);
}


void initializePlayerBitBoard(PlayerBitBoard* playerBitBoard) {
    memset(playerBitBoard, 0, sizeof(PlayerBitBoard));
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


bool hasWinningMove(Board* board, uint16_t smallBoardsWithWinningMove) {
    uint8_t currentBoard = board->state.currentBoard;
    return (currentBoard == ANY_BOARD && smallBoardsWithWinningMove)
           || BIT_CHECK(smallBoardsWithWinningMove, currentBoard);
}


void updateSmallBoardState(Board* board, uint8_t boardIndex) {
    uint16_t p1 = board->state.player1.smallBoards[boardIndex];
    uint16_t p2 = board->state.player2.smallBoards[boardIndex];
    uint16_t mask = 1ULL << boardIndex;
    BIT_CHANGE(board->state.instantWinSmallBoards[PLAYER1], mask, smallBoardHasInstantWinMove(p1, p2));
    BIT_CHANGE(board->state.instantWinSmallBoards[PLAYER2], mask, smallBoardHasInstantWinMove(p2, p1));
}


void updateBigBoardState(Board* board) {
    uint16_t p1 = board->state.player1.bigBoard;
    uint16_t p2 = board->state.player2.bigBoard;
    board->state.instantWinBoards[PLAYER1] = calculateInstantWinBoards(p1, p2);
    board->state.instantWinBoards[PLAYER2] = calculateInstantWinBoards(p2, p1);
}
// END SMART_ROLLOUT










// START ROLLOUT
Square generateMove(Board* board, RNG* rng) {
    uint8_t currentBoard = board->state.currentBoard;
    uint8_t randomMoveIndex;
    if (currentBoard == ANY_BOARD) {
        randomMoveIndex = generateBoundedRandomNumber(rng, board->state.totalAmountOfOpenSquares);
        currentBoard = 0;
        while (currentBoard < 9 && randomMoveIndex - board->state.amountOfOpenSquaresBySmallBoard[currentBoard] >= 0) {
            randomMoveIndex -= board->state.amountOfOpenSquaresBySmallBoard[currentBoard++];
        }
    } else {
        randomMoveIndex = generateBoundedRandomNumber(rng, board->state.amountOfOpenSquaresBySmallBoard[currentBoard]);
    }
    uint16_t bitBoard = ~(board->state.player1.smallBoards[currentBoard] | board->state.player2.smallBoards[currentBoard]) & 511;
    return openSquares[bitBoard][currentBoard][randomMoveIndex];
}


void makeRandomTemporaryMove(Board* board, RNG* rng) {
    Player player = getCurrentPlayer(board);
    uint16_t smallBoardsWithWinningMove = board->state.instantWinBoards[player] & board->state.instantWinSmallBoards[player];
    if (hasWinningMove(board, smallBoardsWithWinningMove)) {
        board->state.winner = board->state.currentPlayer + 1;
        return;
    }
    Square move = generateMove(board, rng);
    makeTemporaryMove(board, move);
}


Winner rollout(Board* board, RNG* rng) {
    while (getWinner(board) == NONE) {
        makeRandomTemporaryMove(board, rng);
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
    initializePlayerBitBoard(&board->state.player1);
    initializePlayerBitBoard(&board->state.player2);
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
    uint16_t bitBoard = ~(board->state.player1.smallBoards[boardIndex] | board->state.player2.smallBoards[boardIndex]) & 511;
    *amountOfMoves = amountOfOpenSquares[bitBoard];
    return openSquares[bitBoard][boardIndex];
}


int8_t copyMovesSingleBoard(Board* board, uint8_t boardIndex, Square moves[TOTAL_SMALL_SQUARES], int8_t amountOfMoves) {
    uint16_t bitBoard = ~(board->state.player1.smallBoards[boardIndex] | board->state.player2.smallBoards[boardIndex]) & 511;
    memcpy(&moves[amountOfMoves], openSquares[bitBoard][boardIndex],
           amountOfOpenSquares[bitBoard] * sizeof(Square));
    return (int8_t)(amountOfMoves + amountOfOpenSquares[bitBoard]);
}


int8_t generateMovesAnyBoard(Board* board, Square moves[TOTAL_SMALL_SQUARES]) {
    int8_t amountOfMoves = 0;
    uint16_t undecidedSmallBoards = ~(board->state.player1.bigBoard | board->state.player2.bigBoard) & 511;
    while (undecidedSmallBoards) {
        uint8_t boardIndex = __builtin_ffs(undecidedSmallBoards) - 1;
        amountOfMoves = copyMovesSingleBoard(board, boardIndex, moves, amountOfMoves);
        undecidedSmallBoards &= undecidedSmallBoards - 1;
    }
    return amountOfMoves;
}


Square* generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves) {
    if (board->state.winner != NONE) {
        *amountOfMoves = 0;
        return moves;
    }
    uint8_t currentBoard = board->state.currentBoard;
    if (currentBoard == ANY_BOARD) {
        *amountOfMoves = generateMovesAnyBoard(board, moves);
        return moves;
    }
    return getMovesSingleBoard(board, currentBoard, amountOfMoves);
}


uint8_t getNextBoard(Board* board, uint8_t previousPosition) {
    bool smallBoardIsDecided = BIT_CHECK(board->state.player1.bigBoard | board->state.player2.bigBoard, previousPosition);
    return smallBoardIsDecided ? ANY_BOARD : previousPosition;
}


bool nextBoardIsEmpty(Board* board) {
    uint8_t currentBoard = board->state.currentBoard;
    return currentBoard != ANY_BOARD
           && (board->state.player1.smallBoards[currentBoard] | board->state.player2.smallBoards[currentBoard]) == 0;
}


bool nextBoardHasOneMoveFromBothPlayers(Board* board) {
    uint8_t currentBoard = board->state.currentBoard;
    return currentBoard != ANY_BOARD
           && ONE_BIT_SET(board->state.player1.smallBoards[currentBoard])
           && ONE_BIT_SET(board->state.player2.smallBoards[currentBoard]);
}


uint8_t getCurrentBoard(Board* board) {
    return board->state.currentBoard;
}


Player getCurrentPlayer(Board* board) {
    return board->state.currentPlayer;
}


void revertToCheckpoint(Board* board) {
    board->state = board->stateCheckpoint;
}


void updateCheckpoint(Board* board) {
    board->stateCheckpoint = board->state;
}


void makeTemporaryMove(Board* board, Square square) {
    PlayerBitBoard* p1 = &board->state.player1;
    if (setSquareOccupied(p1 + board->state.currentPlayer, p1 + !board->state.currentPlayer, square)) {
        board->state.winner = winnerByBigBoards[board->state.player1.bigBoard][board->state.player2.bigBoard];
        board->state.totalAmountOfOpenSquares -= board->state.amountOfOpenSquaresBySmallBoard[square.board];
        board->state.amountOfOpenSquaresBySmallBoard[square.board] = 0;
        updateBigBoardState(board);
    } else {
        board->state.totalAmountOfOpenSquares--;
        board->state.amountOfOpenSquaresBySmallBoard[square.board]--;
    }
    updateSmallBoardState(board, square.board);
    board->state.currentPlayer ^= 1;
    board->state.currentBoard = getNextBoard(board, square.position);
    board->state.ply++;
}


void makePermanentMove(Board* board, Square square) {
    makeTemporaryMove(board, square);
    updateCheckpoint(board);
}


Winner getWinnerAfterMove(Board* board, Square square) {
    State tempCheckpoint = board->state;
    makeTemporaryMove(board, square);
    Winner winner = getWinner(board);
    board->state = tempCheckpoint;
    return winner;
}


Winner getWinner(Board* board) {
    return board->state.winner;
}


void setMe(Board* board, Player player) {
    board->me = player;
}


bool currentPlayerIsMe(Board* board) {
    return board->state.currentPlayer == board->me;
}


uint8_t getPly(Board* board) {
    return board->state.ply;
}
// END BOARD















// START MCTS_NODE
MCTSNode* createMCTSRootNode() {
    MCTSNode* root = calloc(1, sizeof(MCTSNode));
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
    MCTSNode* copy = malloc(sizeof(MCTSNode));
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
    node->children = malloc(sizeof(MCTSNode));
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
            node->children = malloc(amountOfMoves * sizeof(MCTSNode));
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


float fastSquareRoot(float x) {
    __m128 in = _mm_set_ss(x);
    return _mm_cvtss_f32(_mm_mul_ss(in, _mm_rsqrt_ss(in)));
}


#define EXPLORATION_PARAMETER 0.41f
float getUCTValue(MCTSNode* node, float parentLogSims) {
    float exploitation = node->wins*node->simsInverted;
    float exploration = fastSquareRoot(parentLogSims * node->simsInverted);
    return exploitation + exploration;
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
    if (node->amountOfUntriedMoves > 0 && highestUCT < FIRST_PLAY_URGENCY) {
        highestUCTChild = expandNextChild(node);
    }
    return highestUCTChild;
}


MCTSNode* expandNextChild(MCTSNode* node) {
    node->amountOfUntriedMoves--;
    return &node->children[node->amountOfChildren++];
}


MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square) {
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
    } else {
        newRoot->parent = NULL;
        newRoot = copyMCTSNode(newRoot);
    }
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


Square getMostPromisingMove(MCTSNode* node) {
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
// END MCTS_NODE
















// START PRIORS
bool pairPriors[512][512][9];
void calculatePairPriors(uint16_t smallBoard, uint16_t otherPlayerSmallBoard, bool* result) {
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
            if ((smallBoard | (1 << position)) == mask && !isWin(mask | otherPlayerSmallBoard)) {
                result[position] = true;
                break;
            }
        }
    }
}


void initializePriorsLookupTable() {
    for (uint8_t position1 = 0; position1 < 9; position1++) {
        for (uint8_t position2 = 0; position2 < 9; position2++) {
            if (position1 != position2) {
                uint16_t smallBoard = 1 << position1;
                uint16_t otherPlayerSmallBoard = 1 << position2;
                calculatePairPriors(smallBoard, otherPlayerSmallBoard, pairPriors[smallBoard][otherPlayerSmallBoard]);
            }
        }
    }
}


bool* getPairPriors(uint16_t smallBoard, uint16_t otherPlayerSmallBoard) {
    return pairPriors[smallBoard][otherPlayerSmallBoard];
}


void applyPairPriors(MCTSNode* parent, uint16_t smallBoard, uint16_t otherPlayerSmallBoard, float prior) {
    bool* formsPair = pairPriors[smallBoard][otherPlayerSmallBoard];
    for (int i = 0; i < parent->amountOfUntriedMoves; i++) {
        MCTSNode* child = &parent->children[i];
        child->wins = formsPair[child->square.position]? prior : 0.0f;
        child->sims = prior;
        child->simsInverted = 1.0f / prior;
    }
}


void applySendToDecidedBoardPriors(Board* board, MCTSNode* parent, float prior) {
    for (int i = 0; i < parent->amountOfUntriedMoves; i++) {
        MCTSNode* child = &parent->children[i];
        if ((board->state.player1.bigBoard | board->state.player2.bigBoard) & (1 << child->square.position)) {
            child->wins = 0.0f;
            child->sims = prior;
            child->simsInverted = 1.0f / prior;
        }
    }
}


void applyPriors(Board* board, MCTSNode* parent) {
    PlayerBitBoard* p1 = &board->state.player1;
    uint16_t smallBoard = (p1 + board->state.currentPlayer)->smallBoards[board->state.currentBoard];
    uint16_t otherPlayerSmallBoard = (p1 + !board->state.currentPlayer)->smallBoards[board->state.currentBoard];
    uint8_t ply = getPly(board);
    if (nextBoardHasOneMoveFromBothPlayers(board) && ply <= 30) {
        applyPairPriors(parent, smallBoard, otherPlayerSmallBoard, ply <= 18? 1000.0f : 2.0f);
    }
    if (ply <= 40) {
        applySendToDecidedBoardPriors(board, parent, ply <= 30? 1000.0f : 5.0f);
    }
}
// END PRIORS


















// START SOLVER
#define BIG_FLOAT 5000000.0f


void checkAllSet(MCTSNode* node) {
    if (node == NULL || node->amountOfUntriedMoves > 0) {
        return;
    }
    bool hasDraw = false;
    for (int i = 0; i < node->amountOfChildren; i++) {
        MCTSNode child = node->children[i];
        if (child.sims < BIG_FLOAT || child.wins == child.sims) {
            return;
        }
        if (!hasDraw && child.wins > 0) {
            hasDraw = true;
        }
    }
    setNodeWinner(node, hasDraw? DRAW : WIN_P1, PLAYER1);
}


void setNodeWinner(MCTSNode* node, Winner winner, Player player) {
    if (node == NULL || node->sims >= BIG_FLOAT) {
        return;
    }
    node->sims = BIG_FLOAT;
    node->simsInverted = 1.0f / BIG_FLOAT;
    if (winner == DRAW) {
        node->wins = BIG_FLOAT / 2.0f;
        checkAllSet(node->parent);
    } else if (player + 1 == winner) {
        node->wins = BIG_FLOAT;
        setNodeWinner(node->parent, winner, OTHER_PLAYER(player));
    } else {
        node->wins = 0;
        checkAllSet(node->parent);
    }
}
// END SOLVER


















// START FIND_NEXT_MOVE
MCTSNode* selectLeaf(Board* board, MCTSNode* root, RNG* rng) {
    MCTSNode* currentNode = root;
    while (!isLeafNode(currentNode, board, rng) && getWinner(board) == NONE) {
        currentNode = selectNextChild(currentNode);
        visitNode(currentNode, board);
    }
    return currentNode;
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
        MCTSNode* leaf = selectLeaf(board, root, rng);
        Winner winner = getWinner(board);
        Player player = OTHER_PLAYER(getCurrentPlayer(board));
        if (winner == NONE) {
            winner = rollout(board, rng);
        } else {
            setNodeWinner(leaf, winner, player);
        }
        backpropagate(leaf, winner, player);
        revertToCheckpoint(board);
    }
    return amountOfSimulations;
}
// END FIND_NEXT_MOVE

















// START HANDLE_TURN
Square handleOpening(Board* board) {
    uint8_t ply = getPly(board);
    Square invalid = {9, 9};
    if (board->me != PLAYER2 || ply > 10) {
        return invalid;
    }
    bool firstMove = BIT_CHECK(board->state.player1.smallBoards[4], 4);
    bool secondMove = BIT_CHECK(board->state.player1.smallBoards[8], 8);
    bool thirdMove = BIT_CHECK(board->state.player1.smallBoards[0], 0);
    bool fourthMove = BIT_CHECK(board->state.player1.smallBoards[5], 5);
    bool fifthMove = BIT_CHECK(board->state.player1.smallBoards[7], 7);
    uint8_t currentBoard = getCurrentBoard(board);
    if (ply == 1 && firstMove) {
        Square move = {4, 8};
        return move;
    } else if (ply == 3 && firstMove && secondMove && currentBoard == 8) {
        Square move = {8, 0};
        return move;
    } else if (ply == 5 && firstMove && secondMove && thirdMove && currentBoard == 0) {
        Square move = {0, 5};
        return move;
    } else if (ply == 7 && firstMove && secondMove && thirdMove && fourthMove && currentBoard == 5) {
        Square move = {5, 7};
        return move;
    } else if (ply == 9 && firstMove && secondMove && thirdMove && fourthMove && fifthMove && currentBoard == 7) {
        Square move = {7, 5};
        return move;
    }
    return invalid;
}


MCTSNode* handleEnemyTurn(Board* board, MCTSNode* root, Square enemyMove, RNG* rng) {
    if (enemyMove.board == 9 && enemyMove.position == 9) {
        setMe(board, PLAYER1);
        return root;
    }
    discoverChildNodes(root, board, rng);
    MCTSNode* newRoot = updateRoot(root, board, enemyMove);
    makePermanentMove(board, enemyMove);
    return newRoot;
}


HandleTurnResult handleTurn(Board* board, MCTSNode* root, RNG* rng, double allocatedTime, Square enemyMove) {
    root = handleEnemyTurn(board, root, enemyMove, rng);
    Square openingMove = handleOpening(board);
    if (openingMove.board != 9) {
        MCTSNode* newRoot = updateRoot(root, board, openingMove);
        makePermanentMove(board, openingMove);
        HandleTurnResult result = {openingMove, newRoot, 0};
        return result;
    }
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


#define TIME 0.090
int main() {
    // runTests();
    playGame(stdin, TIME);
}
// END MAIN