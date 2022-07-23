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

Square createSquare2(uint8_t board, uint8_t position);

bool squaresAreEqual2(Square square1, Square square2);

#define BIT_SET2(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CHECK2(a,b) ((a) & (1ULL<<(b)))

void crash2(char* errorMessage);

Square toOurNotation2(Square rowAndColumn);

Square toGameNotation2(Square square);

typedef struct pcg32_random_t {
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

void pcg32_srandom_r2(pcg32_random_t* rng, uint64_t init_state, uint64_t init_seq);

uint8_t pcg32_boundedrand_r2(pcg32_random_t* rng, uint8_t bound);

typedef enum Player {
    PLAYER1,
    PLAYER2
} Player;

Player otherPlayer2(Player player);

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

bool playerIsWinner2(Player player, Winner winner);

typedef struct PlayerBitBoard PlayerBitBoard;

PlayerBitBoard* createPlayerBitBoard2();

void freePlayerBitBoard2(PlayerBitBoard* playerBitBoard);

uint16_t getBigBoard2(PlayerBitBoard* playerBitBoard);

bool boardIsWon2(PlayerBitBoard* playerBitBoard, uint8_t board);

uint16_t getSmallBoard2(PlayerBitBoard* playerBitBoard, uint8_t board);

bool squareIsOccupied2(PlayerBitBoard* playerBitBoard, Square square);

bool isWin2(uint16_t smallBoard);

bool setSquareOccupied2(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayer2BitBoard, Square square);

void revertToPlayerCheckpoint2(PlayerBitBoard* playerBitBoard);

void updatePlayerCheckpoint2(PlayerBitBoard* playerBitBoard);

#define TOTAL_SMALL_SQUARES 81

typedef struct Board Board;

Board* createBoard2();

void freeBoard2(Board* board);

Square* generateMoves2(Board* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves);

void makeRandomTemporaryMove2(Board* board, pcg32_random_t* rng);

bool nextBoardIsEmpty2(Board* board);

uint8_t getCurrentBoard2(Board* board);

void revertToCheckpoint2(Board* board);

void makeTemporaryMove2(Board* board, Square square);

void makePermanentMove2(Board* board, Square square);

Winner getWinner2(Board* board);

void setMe2(Board* board, Player player);

bool currentPlayerIsMe2(Board* board);

uint8_t getPly2(Board* board);

typedef struct MCTSNode MCTSNode;

MCTSNode* createMCTSRootNode2();

void freeMCTSTree2(MCTSNode* node);

bool isLeafNode2(MCTSNode* node, Board* board);

MCTSNode* selectNextChild2(MCTSNode* node, Board* board);

MCTSNode* updateRoot2(MCTSNode* root, Board* board, Square square);

void backpropagate2(MCTSNode* node, Winner winner);

void visitNode2(MCTSNode* node, Board* board);

void setNodeWinner2(MCTSNode* node, Winner winner);

Square getMostPromisingMove2(MCTSNode* node, Board* board);

int getSims2(MCTSNode* node);

float getWinrate2(MCTSNode* node);

int findNextMove2(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime);

typedef struct HandleTurnResult {
    Square move;
    MCTSNode* newRoot;
    int amountOfSimulations;
} HandleTurnResult;

HandleTurnResult handleTurn2(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime, Square enemyMove);

// END HEADERS












// START SQUARE
Square createSquare2(uint8_t board, uint8_t position) {
    Square result = {board, position};
    return result;
}


bool squaresAreEqual2(Square square1, Square square2) {
    return square1.board == square2.board && square1.position == square2.position;
}
// END SQUARE














// START UTIL
void crash2(char* errorMessage) {
    fprintf(stderr, "%s\n", errorMessage);
    exit(1);
}


Square toOurNotation2(Square rowAndColumn) {
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


Square toGameNotation2(Square square) {
    uint8_t row = (square.position / 3) + 3*(square.board / 3);
    uint8_t col = (square.position % 3) + 3*(square.board % 3);
    Square result = {row, col};
    return result;
}
// END UTIL












// START RANDOM
uint32_t pcg32_random_r2(pcg32_random_t* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}


void pcg32_srandom_r2(pcg32_random_t* rng, uint64_t init_state, uint64_t init_seq) {
    rng->state = 0U;
    rng->inc = (init_seq << 1u) | 1u;
    pcg32_random_r2(rng);
    rng->state += init_state;
    pcg32_random_r2(rng);
}


uint8_t pcg32_boundedrand_r2(pcg32_random_t* rng, uint8_t bound) {
    return pcg32_random_r2(rng) % bound;
}
// END RANDOM











// START PLAYER
Player otherPlayer2(Player player) {
    return player == PLAYER1? PLAYER2 : PLAYER1;
}


bool playerIsWinner2(Player player, Winner winner) {
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


PlayerBitBoard* createPlayerBitBoard2() {
    return calloc(1, sizeof(PlayerBitBoard));
}


void freePlayerBitBoard2(PlayerBitBoard* playerBitBoard) {
    free(playerBitBoard);
}


uint16_t getBigBoard2(PlayerBitBoard* playerBitBoard) {
    return playerBitBoard->bigBoard;
}


bool boardIsWon2(PlayerBitBoard* playerBitBoard, uint8_t board) {
    return BIT_CHECK2(playerBitBoard->bigBoard, board);
}


uint16_t getSmallBoard2(PlayerBitBoard* playerBitBoard, uint8_t board) {
    return playerBitBoard->smallBoards[board];
}


bool squareIsOccupied2(PlayerBitBoard* playerBitBoard, Square square) {
    return BIT_CHECK2(playerBitBoard->smallBoards[square.board], square.position);
}


bool isWin2(uint16_t smallBoard) {
    const __m128i masks = _mm_setr_epi16(0x7, 0x38, 0x1c0, 0x49, 0x92, 0x124, 0x111, 0x54);
    const __m128i v1 = _mm_set1_epi16(1);
    __m128i boards128 = _mm_set1_epi16((short) smallBoard);
    __m128i andResult = _mm_and_si128(masks, boards128);
    __m128i result = _mm_cmpeq_epi16(andResult, masks);
    return !_mm_test_all_zeros(result, v1);
}


bool isDraw2(uint16_t smallBoard, uint16_t otherPlayer2SmallBoard) {
    return (smallBoard | otherPlayer2SmallBoard) == 511;
}


bool setSquareOccupied2(PlayerBitBoard* playerBitBoard, PlayerBitBoard* otherPlayer2BitBoard, Square square) {
    BIT_SET2(playerBitBoard->smallBoards[square.board], square.position);
    uint16_t smallBoard = playerBitBoard->smallBoards[square.board];
    uint16_t otherPlayer2SmallBoard = otherPlayer2BitBoard->smallBoards[square.board];
    if (isWin2(smallBoard)) {
        BIT_SET2(playerBitBoard->bigBoard, square.board);
        return true;
    } if (isDraw2(smallBoard, otherPlayer2SmallBoard)) {
        BIT_SET2(playerBitBoard->bigBoard, square.board);
        BIT_SET2(otherPlayer2BitBoard->bigBoard, square.board);
        return true;
    }
    return false;
}


void revertToPlayerCheckpoint2(PlayerBitBoard* playerBitBoard) {
    for (int i = 0; i < 9; i++) {
        playerBitBoard->smallBoards[i] = playerBitBoard->checkpointSmallBoards[i];
    }
    playerBitBoard->bigBoard = playerBitBoard->checkpointBigBoard;
}


void updatePlayerCheckpoint2(PlayerBitBoard* playerBitBoard) {
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


int8_t setOpenSquares2(Square openSquares[9], uint8_t boardIndex, uint16_t bitBoard) {
    int8_t amountOfMoves = 0;
    while (bitBoard) {
        Square square = {boardIndex, __builtin_ffs(bitBoard) - 1};
        openSquares[amountOfMoves++] = square;
        bitBoard &= bitBoard - 1;
    }
    return amountOfMoves;
}


Board* createBoard2() {
    Board* board = malloc(sizeof(Board));
    board->player1 = createPlayerBitBoard2();
    board->player2 = createPlayerBitBoard2();
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
            board->amountOfOpenSquares[bitBoard] =
                    setOpenSquares2(board->openSquares[bitBoard][boardIndex], boardIndex, bitBoard);
        }
    }
    board->me = PLAYER2;
    return board;
}


void freeBoard2(Board* board) {
    freePlayerBitBoard2(board->player1);
    freePlayerBitBoard2(board->player2);
    free(board);
}


Square* getMovesSingleBoard2(Board* board, uint8_t boardIndex, int8_t* amountOfMoves) {
    uint16_t bitBoard = ~(getSmallBoard2(board->player1, boardIndex) | getSmallBoard2(board->player2, boardIndex)) & 511;
    *amountOfMoves = board->amountOfOpenSquares[bitBoard];
    return board->openSquares[bitBoard][boardIndex];
}


int8_t copyMovesSingleBoard2(Board* board, uint8_t boardIndex, Square moves[TOTAL_SMALL_SQUARES], int8_t amountOfMoves) {
    uint16_t bitBoard = ~(getSmallBoard2(board->player1, boardIndex) | getSmallBoard2(board->player2, boardIndex)) & 511;
    memcpy(&moves[amountOfMoves], board->openSquares[bitBoard][boardIndex],
           board->amountOfOpenSquares[bitBoard] * sizeof(Square));
    return (int8_t)(amountOfMoves + board->amountOfOpenSquares[bitBoard]);
}


int8_t generateMoves2AnyBoard(Board* board, Square moves[TOTAL_SMALL_SQUARES]) {
    int8_t amountOfMoves = 0;
    uint16_t undecidedSmallBoards = ~(getBigBoard2(board->player1) | getBigBoard2(board->player2)) & 511;
    while (undecidedSmallBoards) {
        uint8_t boardIndex = __builtin_ffs(undecidedSmallBoards) - 1;
        amountOfMoves = copyMovesSingleBoard2(board, boardIndex, moves, amountOfMoves);
        undecidedSmallBoards &= undecidedSmallBoards - 1;
    }
    return amountOfMoves;
}


Square* generateMoves2(Board* board, Square moves[TOTAL_SMALL_SQUARES], int8_t* amountOfMoves) {
    if (board->AS.winner != NONE) {
        *amountOfMoves = 0;
        return moves;
    }
    uint8_t currentBoard = board->AS.currentBoard;
    if (currentBoard == ANY_BOARD) {
        *amountOfMoves = generateMoves2AnyBoard(board, moves);
        return moves;
    }
    return getMovesSingleBoard2(board, currentBoard, amountOfMoves);
}


void makeRandomTemporaryMove2(Board* board, pcg32_random_t* rng) {
    uint8_t currentBoard = board->AS.currentBoard;
    uint8_t randomMoveIndex;
    if (currentBoard == ANY_BOARD) {
        uint8_t randomMultipleBoardMoveIndex = pcg32_boundedrand_r2(rng, board->AS.totalAmountOfOpenSquares);
        int boardIndex = 0;
        int cumulativeSum = 0;
        while (boardIndex < 9 && cumulativeSum <= randomMultipleBoardMoveIndex) {
            cumulativeSum += board->AS.amountOfOpenSquaresBySmallBoard[boardIndex++];
        }
        cumulativeSum -= board->AS.amountOfOpenSquaresBySmallBoard[--boardIndex];
        randomMoveIndex = randomMultipleBoardMoveIndex - cumulativeSum;
        currentBoard = boardIndex;
    } else {
        randomMoveIndex = pcg32_boundedrand_r2(rng, board->AS.amountOfOpenSquaresBySmallBoard[currentBoard]);
    }
    int8_t amountOfMoves;
    Square* moves = getMovesSingleBoard2(board, currentBoard, &amountOfMoves);
    Square randomMove = moves[randomMoveIndex];
    makeTemporaryMove2(board, randomMove);
}


bool nextBoardIsEmpty2(Board* board) {
    uint8_t currentBoard = board->AS.currentBoard;
    return currentBoard != ANY_BOARD
           && (getSmallBoard2(board->player1, currentBoard) | getSmallBoard2(board->player2, currentBoard)) == 0;
}


uint8_t getCurrentBoard2(Board* board) {
    return board->AS.currentBoard;
}


void revertToCheckpoint2(Board* board) {
    revertToPlayerCheckpoint2(board->player1);
    revertToPlayerCheckpoint2(board->player2);
    board->AS = board->ASCheckpoint;
}


void updateCheckpoint2(Board* board) {
    updatePlayerCheckpoint2(board->player1);
    updatePlayerCheckpoint2(board->player2);
    board->ASCheckpoint = board->AS;
}


Winner getSmallBoard2Winner(Board* board, uint8_t boardIndex) {
    bool player1Bit = boardIsWon2(board->player1, boardIndex);
    bool player2Bit = boardIsWon2(board->player2, boardIndex);
    return 2*player2Bit + player1Bit;
}


Occupation getSquare2(Board* board, Square square) {
    bool player1Bit = squareIsOccupied2(board->player1, square);
    bool player2Bit = squareIsOccupied2(board->player2, square);
    return 2*player2Bit + player1Bit;
}


uint8_t getNextBoard2(Board* board, uint8_t previousPosition) {
    Winner smallBoardWinner = getSmallBoard2Winner(board, previousPosition);
    return smallBoardWinner == NONE ? previousPosition : ANY_BOARD;
}


Winner calculateWinner2(Board* board) {
    uint16_t player1BigBoard = getBigBoard2(board->player1);
    uint16_t player2BigBoard = getBigBoard2(board->player2);
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


void makeTemporaryMove2(Board* board, Square square) {
    bool bigBoardWasUpdated = board->AS.currentPlayer == PLAYER1
                              ? setSquareOccupied2(board->player1, board->player2, square)
                              : setSquareOccupied2(board->player2, board->player1, square);
    if (bigBoardWasUpdated) {
        board->AS.winner = calculateWinner2(board);
        board->AS.totalAmountOfOpenSquares -= board->AS.amountOfOpenSquaresBySmallBoard[square.board];
        board->AS.amountOfOpenSquaresBySmallBoard[square.board] = 0;
    } else {
        board->AS.totalAmountOfOpenSquares--;
        board->AS.amountOfOpenSquaresBySmallBoard[square.board]--;
    }
    board->AS.currentPlayer = otherPlayer2(board->AS.currentPlayer);
    board->AS.currentBoard = getNextBoard2(board, square.position);
    board->AS.ply++;
}


void makePermanentMove2(Board* board, Square square) {
    makeTemporaryMove2(board, square);
    updateCheckpoint2(board);
}


Winner getWinner2(Board* board) {
    return board->AS.winner;
}


void setMe2(Board* board, Player player) {
    board->me = player;
}


bool currentPlayerIsMe2(Board* board) {
    return board->AS.currentPlayer == board->me;
}


uint8_t getPly2(Board* board) {
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


MCTSNode* createMCTSRootNode2() {
    MCTSNode* root = calloc(1, sizeof(MCTSNode));
    root->amountOfChildren = -1;
    root->player = PLAYER2;
    root->square.board = 9;
    root->square.position = 9;
    root->amountOfUntriedMoves = -1;
    return root;
}


MCTSNode* createMCTSNode2(MCTSNode* parent, Square square) {
    MCTSNode* node = calloc(1, sizeof(MCTSNode));
    node->parent = parent;
    node->amountOfChildren = -1;
    node->player = otherPlayer2(parent->player);
    node->square = square;
    node->amountOfUntriedMoves = -1;
    return node;
}


void freeNode2(MCTSNode* node) {
    free(node->children);
    free(node->untriedMoves);
    free(node);
}


void freeMCTSTree2(MCTSNode* node) {
    for (int i = 0; i < node->amountOfChildren; i++) {
        freeMCTSTree2(node->children[i]);
    }
    freeNode2(node);
}


void discoverChildNodes2(MCTSNode* node, Board* board) {
    if (node->amountOfChildren == -1) {
        node->amountOfChildren = 0;
        if (nextBoardIsEmpty2(board) && currentPlayerIsMe2(board) && getPly2(board) <= 20) {
            node->amountOfUntriedMoves = 1;
            node->untriedMoves = malloc(sizeof(Square));
            uint8_t currentBoard = getCurrentBoard2(board);
            Square sameBoard = {currentBoard, currentBoard};
            node->untriedMoves[0] = sameBoard;
        } else {
            Square movesArray[TOTAL_SMALL_SQUARES];
            int8_t amountOfMoves;
            Square* moves = generateMoves2(board, movesArray, &amountOfMoves);
            node->amountOfUntriedMoves = amountOfMoves;
            node->untriedMoves = malloc(amountOfMoves * sizeof(Square));
            for (int i = 0; i < amountOfMoves; i++) {
                node->untriedMoves[i] = moves[i];
            }
        }
    }
}


bool isLeafNode2(MCTSNode* node, Board* board) {
    discoverChildNodes2(node, board);
    return node->amountOfUntriedMoves > 0;
}


#define EXPLORATION_PARAMETER 0.5f
float getUCTValue2(MCTSNode* node, float parentLogSims) {
    if (node->UCTValue) {
        return node->UCTValue;
    }
    float w = node->wins;
    float n = node->sims;
    float c = EXPLORATION_PARAMETER;
    return w/n + c*sqrtf(parentLogSims / n);
}


MCTSNode* expandNode2(MCTSNode* node, int childIndex) {
    MCTSNode* newChild = createMCTSNode2(node, node->untriedMoves[childIndex]);
    node->amountOfUntriedMoves--;
    node->children = realloc(node->children, (node->amountOfChildren + 1) * sizeof(MCTSNode*));
    node->children[node->amountOfChildren++] = newChild;
    return newChild;
}


MCTSNode* selectNextChild2(MCTSNode* node, Board* board) {
    discoverChildNodes2(node, board);
    if (node->amountOfUntriedMoves) {
        return expandNode2(node, node->amountOfUntriedMoves - 1);
    }
    float logSims = logf(node->sims);
    MCTSNode* highestUCTChild = node->children[0];
    float highestUCT = getUCTValue2(highestUCTChild, logSims);
    for (int i = 1; i < node->amountOfChildren; i++) {
        MCTSNode* child = node->children[i];
        float UCT = getUCTValue2(child, logSims);
        if (UCT > highestUCT) {
            highestUCTChild = child;
            highestUCT = UCT;
        }
    }
    return highestUCTChild;
}


MCTSNode* updateRoot2(MCTSNode* root, Board* board, Square square) {
    discoverChildNodes2(root, board);
    MCTSNode* newRoot = NULL;
    for (int i = 0; i < root->amountOfChildren; i++) {
        MCTSNode* child = root->children[i];
        if (squaresAreEqual2(square, child->square)) {
            child->parent = NULL;
            newRoot = child;
        } else {
            freeMCTSTree2(child);
        }
    }
    if (newRoot == NULL) {
        for (int i = 0; i < root->amountOfUntriedMoves; i++) {
            if (squaresAreEqual2(square, root->untriedMoves[i])) {
                newRoot = expandNode2(root, i);
                newRoot->parent = NULL;
                break;
            }
        }
    }
    freeNode2(root);
    return newRoot;
}


void backpropagate2(MCTSNode* node, Winner winner) {
    node->sims++;
    if (playerIsWinner2(node->player, winner)) {
        node->wins++;
    } else if (winner == DRAW) {
        node->wins += 0.5f;
    }
    if (node->parent != NULL) {
        backpropagate2(node->parent, winner);
    }
}


void visitNode2(MCTSNode* node, Board* board) {
    makeTemporaryMove2(board, node->square);
}


#define UCT_WIN 100000
#define UCT_LOSS (-UCT_WIN)
void setNodeWinner2(MCTSNode* node, Winner winner) {
    if (winner != DRAW) {
        bool win = playerIsWinner2(node->player, winner);
        node->UCTValue = win? UCT_WIN : UCT_LOSS;
        if (!win) {
            node->parent->UCTValue = UCT_WIN;
        }
    }
}


Square getMostPromisingMove2(MCTSNode* node, Board* board) {
    discoverChildNodes2(node, board);
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


int getSims2(MCTSNode* node) {
    return (int) node->sims;
}


float getWinrate2(MCTSNode* node) {
    return node->wins / node->sims;
}
// END MCTS_NODE

















// START FIND_NEXT_MOVE
MCTSNode* selectLeaf2(Board* board, MCTSNode* root) {
    MCTSNode* currentNode = root;
    while (!isLeafNode2(currentNode, board) && getWinner2(board) == NONE) {
        currentNode = selectNextChild2(currentNode, board);
        visitNode2(currentNode, board);
    }
    return currentNode;
}


MCTSNode* expandLeaf2(Board* board, MCTSNode* leaf) {
    MCTSNode* nextChild = selectNextChild2(leaf, board);
    visitNode2(nextChild, board);
    return nextChild;
}


Winner simulate2(Board* board, pcg32_random_t* rng) {
    while (getWinner2(board) == NONE) {
        makeRandomTemporaryMove2(board, rng);
    }
    Winner simulationWinner = getWinner2(board);
    revertToCheckpoint2(board);
    return simulationWinner;
}


clock_t getDeadline2(double time) {
    return clock() + (clock_t)(time*CLOCKS_PER_SEC);
}


bool hasTimeRemaining2(clock_t deadline) {
    return clock() < deadline;
}


int findNextMove2(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime) {
    clock_t deadline = getDeadline2(allocatedTime);
    int amountOfSimulations = 0;
    while (++amountOfSimulations % 128 != 0 || hasTimeRemaining2(deadline)) {
        MCTSNode* leaf = selectLeaf2(board, root);
        MCTSNode* playoutNode;
        Winner simulationWinner;
        Winner winner = getWinner2(board);
        if (winner == NONE) {
            playoutNode = expandLeaf2(board, leaf);
            simulationWinner = simulate2(board, rng);
        } else {
            playoutNode = leaf;
            simulationWinner = winner;
            setNodeWinner2(playoutNode, winner);
            revertToCheckpoint2(board);
        }
        backpropagate2(playoutNode, simulationWinner);
    }
    return amountOfSimulations;
}
// END FIND_NEXT_MOVE

















// START HANDLE_TURN
MCTSNode* handleEnemyTurn2(Board* board, MCTSNode* root, Square enemyMove) {
    if (enemyMove.board == 9 && enemyMove.position == 9) {
        setMe2(board, PLAYER1);
        return root;
    }
    MCTSNode* newRoot = updateRoot2(root, board, enemyMove);
    makePermanentMove2(board, enemyMove);
    return newRoot;
}


HandleTurnResult handleTurn2(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime, Square enemyMove) {
    root = handleEnemyTurn2(board, root, enemyMove);
    int amountOfSimulations = findNextMove2(board, root, rng, allocatedTime);
    Square move = getMostPromisingMove2(root, board);
    MCTSNode* newRoot = updateRoot2(root, board, move);
    makePermanentMove2(board, move);
    HandleTurnResult result = {move, newRoot, amountOfSimulations};
    return result;
}
// END HANDLE_TURN






















// START MAIN
void skipMovesInput2(FILE* file) {
    int validActionCount;
    fscanf(file, "%d", &validActionCount);  // NOLINT(cert-err34-c)
    for (int i = 0; i < validActionCount; i++) {
        int row;
        int col;
        fscanf(file, "%d%d", &row, &col);  // NOLINT(cert-err34-c)
    }
}


void printMoveOpponent(MCTSNode* root, Square bestMove, int amountOfSimulations) {
    Square s = toGameNotation2(bestMove);
    uint8_t x = s.board;
    uint8_t y = s.position;
    double winrate = getWinrate2(root);
    printf("%d %d %.4f %d\n", x, y, winrate, amountOfSimulations);
    fflush(stdout);
}


typedef struct StateOpponent {
    Board* board;
    MCTSNode* root;
    pcg32_random_t rng;
} StateOpponent;


StateOpponent* initializeStateOpponent() {
    StateOpponent* state = malloc(sizeof(StateOpponent));
    state->board = createBoard2();
    state->root = createMCTSRootNode2();
    pcg32_srandom_r2(&state->rng, 69, 420);
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
    HandleTurnResult result = handleTurn2((*state)->board, (*state)->root, &(*state)->rng, allocatedTime, enemyMove);
    (*state)->root = result.newRoot;
    if (getWinner2((*state)->board) != NONE) {
        freeStateOpponent(*state);
        *state = NULL;
    }
    return result.move;
}


void playGameOpponent(FILE* file, double timePerMove) {
    Board* board = createBoard2();
    MCTSNode* root = createMCTSRootNode2();
    pcg32_random_t rng;
    pcg32_srandom_r2(&rng, time(NULL), 420);
    while (true) {
        int enemy_row;
        int enemy_col;
        int v = fscanf(file, "%d%d", &enemy_row, &enemy_col);  // NOLINT(cert-err34-c)
        if (v == EOF) {
            break;
        }
        skipMovesInput2(file);
        Square enemyMoveGameNotation = {enemy_row, enemy_col};
        Square enemyMove = toOurNotation2(enemyMoveGameNotation);
        HandleTurnResult result = handleTurn2(board, root, &rng, timePerMove, enemyMove);
        root = result.newRoot;
        printMoveOpponent(root, result.move, result.amountOfSimulations);
    }
    freeMCTSTree2(root);
    freeBoard2(board);
}
// END MAIN