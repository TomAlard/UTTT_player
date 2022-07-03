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



// START HEADERS

typedef struct Square {
    uint8_t board;
    uint8_t position;
} Square;

Square createSquare(uint8_t board, uint8_t position);

bool squaresAreEqual(Square square1, Square square2);

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CHECK(a,b) ((a) & (1ULL<<(b)))

void* safe_malloc(size_t size);

void* safe_calloc(size_t size);

void safe_free(void* pointer);

void crash(char* errorMessage);

void assertMsg(bool condition, char* errorMessage);

Square toOurNotation(Square rowAndColumn);

Square toGameNotation(Square square);

typedef struct pcg32_random_t {
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

uint32_t pcg32_random_r(pcg32_random_t* rng);

void pcg32_srandom_r(pcg32_random_t* rng, uint64_t init_state, uint64_t init_seq);

uint32_t pcg32_boundedrand_r(pcg32_random_t* rng, uint32_t bound);

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

int generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES]);

void revertToCheckpoint(Board* board);

void makeTemporaryMove(Board* board, Square square);

void makePermanentMove(Board* board, Square square);

Winner getWinner(Board* board);

typedef struct MCTSNode MCTSNode;

MCTSNode* createMCTSRootNode();

void freeMCTSTree(MCTSNode* root);

bool hasChildren(MCTSNode* node, Board* board);

bool isLeafNode(MCTSNode* node);

MCTSNode* selectNextChild(MCTSNode* node, Board* board);

MCTSNode* updateRoot(MCTSNode* root, Board* board, Square square);

void backpropagate(MCTSNode* node, Winner winner);

void visitNode(MCTSNode* node, Board* board);

void setNodeWinner(MCTSNode* node, Winner winner);

Square getMostSimulatedChildSquare(MCTSNode* node);

int getSims(MCTSNode* node);

double getWinrate(MCTSNode* node);

Square findNextMove(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime);

typedef struct HandleTurnResult {
    Square move;
    MCTSNode* newRoot;
} HandleTurnResult;

HandleTurnResult handleTurn(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime, Square enemyMove);

// END HEADERS












// START SQUARE
Square createSquare(uint8_t board, uint8_t position) {
    assertMsg(board <= 8 && position <= 8, "Square board and position must be between 0 and 8 inclusive");
    Square result = {board, position};
    return result;
}


bool squaresAreEqual(Square square1, Square square2) {
    return square1.board == square2.board && square1.position == square2.position;
}
// END SQUARE














// START UTIL
void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Couldn't allocate %zu bytes of memory!\n", size);
        exit(1);
    }
    return ptr;
}


void* safe_calloc(size_t size) {
    void* ptr = calloc(1, size);
    if (ptr == NULL) {
        fprintf(stderr, "Couldn't allocate %zu bytes of memory!\n", size);
        exit(1);
    }
    return ptr;
}


void safe_free(void* pointer) {
    if (pointer == NULL) {
        fprintf(stderr, "Cannot free NULL pointer!\n");
        exit(1);
    }
    free(pointer);
}


void crash(char* errorMessage) {
    fprintf(stderr, "%s\n", errorMessage);
    exit(1);
}


void assertMsg(bool condition, char* errorMessage) {
    if (!condition) {
        crash(errorMessage);
    }
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


uint32_t pcg32_boundedrand_r(pcg32_random_t* rng, uint32_t bound) {
    uint32_t threshold = -bound % bound;
    while (true) {
        uint32_t r = pcg32_random_r(rng);
        if (r >= threshold) {
            return r % bound;
        }
    }
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
    return safe_calloc(sizeof(PlayerBitBoard));
}


void freePlayerBitBoard(PlayerBitBoard* playerBitBoard) {
    safe_free(playerBitBoard);
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


uint16_t winningMasks[8] = {448, 56, 7, 292, 146, 73, 273, 84};  // in order: 3 horizontal, 3 vertical, 2 diagonal
bool isWin(uint16_t smallBoard) {
    for (int i = 0; i < 8; i++) {
        uint16_t winningMask = winningMasks[i];
        if ((smallBoard & winningMask) == winningMask) {
            return true;
        }
    }
    return false;
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
} AdditionalState;


typedef struct Board {
    PlayerBitBoard* player1;
    PlayerBitBoard* player2;
    AdditionalState additionalState;
    AdditionalState additionalStateCheckpoint;
} Board;


Board* createBoard() {
    Board* board = safe_malloc(sizeof(Board));
    board->player1 = createPlayerBitBoard();
    board->player2 = createPlayerBitBoard();
    board->additionalState.currentPlayer = PLAYER1;
    board->additionalState.currentBoard = ANY_BOARD;
    board->additionalState.winner = NONE;
    board->additionalStateCheckpoint = board->additionalState;
    return board;
}


void freeBoard(Board* board) {
    freePlayerBitBoard(board->player1);
    freePlayerBitBoard(board->player2);
    safe_free(board);
}


int generateMovesSingleBoard(Board* board, uint8_t boardIndex, Square moves[TOTAL_SMALL_SQUARES], int amountOfMoves) {
    uint16_t smallBoardPlayer1 = getSmallBoard(board->player1, boardIndex);
    uint16_t smallBoardPlayer2 = getSmallBoard(board->player2, boardIndex);
    uint16_t openSquares = ~(smallBoardPlayer1 | smallBoardPlayer2) & 511;
    while (openSquares) {
        Square square = {boardIndex, __builtin_ffs(openSquares) - 1};
        moves[amountOfMoves++] = square;
        openSquares &= openSquares - 1;
    }
    return amountOfMoves;
}


int generateMovesAnyBoard(Board* board, Square moves[TOTAL_SMALL_SQUARES]) {
    int amountOfMoves = 0;
    uint16_t undecidedSmallBoards = ~(getBigBoard(board->player1) | getBigBoard(board->player2)) & 511;
    while (undecidedSmallBoards) {
        uint8_t boardIndex = __builtin_ffs(undecidedSmallBoards) - 1;
        amountOfMoves = generateMovesSingleBoard(board, boardIndex, moves, amountOfMoves);
        undecidedSmallBoards &= undecidedSmallBoards - 1;
    }
    return amountOfMoves;
}


int generateMoves(Board* board, Square moves[TOTAL_SMALL_SQUARES]) {
    if (board->additionalState.winner != NONE) {
        return 0;
    }
    uint8_t currentBoard = board->additionalState.currentBoard;
    return currentBoard == ANY_BOARD
           ? generateMovesAnyBoard(board, moves)
           : generateMovesSingleBoard(board, currentBoard, moves, 0);
}


void revertToCheckpoint(Board* board) {
    revertToPlayerCheckpoint(board->player1);
    revertToPlayerCheckpoint(board->player2);
    board->additionalState = board->additionalStateCheckpoint;
}


void updateCheckpoint(Board* board) {
    updatePlayerCheckpoint(board->player1);
    updatePlayerCheckpoint(board->player2);
    board->additionalStateCheckpoint = board->additionalState;
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


void verifyWinner(Board* board) {
    uint16_t player1BigBoard = getBigBoard(board->player1);
    uint16_t player2BigBoard = getBigBoard(board->player2);
    uint16_t decisiveBoards = player1BigBoard ^ player2BigBoard;
    uint16_t boardsWonByPlayer1 = player1BigBoard & decisiveBoards;
    if (isWin(boardsWonByPlayer1)) {
        board->additionalState.winner = WIN_P1;
        return;
    }
    uint16_t boardsWonByPlayer2 = player2BigBoard & decisiveBoards;
    if (isWin(boardsWonByPlayer2)) {
        board->additionalState.winner = WIN_P2;
        return;
    }
    if ((player1BigBoard | player2BigBoard) == 511) {
        int player1AmountBoardsWon = __builtin_popcount(player1BigBoard);
        int player2AmountBoardsWon = __builtin_popcount(player2BigBoard);
        board->additionalState.winner = (
                player1AmountBoardsWon > player2AmountBoardsWon
                ? WIN_P1
                : player1AmountBoardsWon < player2AmountBoardsWon
                  ? WIN_P2
                  : DRAW
        );
    }
}


void makeTemporaryMove(Board* board, Square square) {
    assertMsg(
            square.board == board->additionalState.currentBoard
            || board->additionalState.currentBoard == ANY_BOARD,
            "Can't make a move on that board");
    assertMsg(getSquare(board, square) == UNOCCUPIED, "Can't make a move on a square that is already occupied");
    assertMsg(board->additionalState.winner == NONE, "Can't make a move when there is already a winner");

    bool bigBoardWasUpdated = board->additionalState.currentPlayer == PLAYER1
                              ? setSquareOccupied(board->player1, board->player2, square)
                              : setSquareOccupied(board->player2, board->player1, square);
    if (bigBoardWasUpdated) {
        verifyWinner(board);
    }
    board->additionalState.currentPlayer = otherPlayer(board->additionalState.currentPlayer);
    board->additionalState.currentBoard = getNextBoard(board, square.position);
}


void makePermanentMove(Board* board, Square square) {
    makeTemporaryMove(board, square);
    updateCheckpoint(board);
}


Winner getWinner(Board* board) {
    return board->additionalState.winner;
}

// END BOARD















// START MCTS_NODE
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
    root->player = PLAYER2;
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


#define EXPLORATION_PARAMETER 0.5
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
    assertMsg(newRoot != NULL, "updateRoot: newRoot shouldn't be NULL");
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
    makeTemporaryMove(board, node->square);
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
// END MCTS_NODE

















// START FIND_NEXT_MOVE
MCTSNode* selectLeaf(Board* board, MCTSNode* root) {
    MCTSNode* currentNode = root;
    while (!isLeafNode(currentNode) && hasChildren(currentNode, board)) {
        currentNode = selectNextChild(currentNode, board);
        assertMsg(currentNode != NULL, "selectLeaf: currentNode is NULL!");
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
        Square validMoves[TOTAL_SMALL_SQUARES];
        int amountOfMoves = generateMoves(board, validMoves);
        Square move = validMoves[pcg32_boundedrand_r(rng, amountOfMoves)];
        makeTemporaryMove(board, move);
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


Square findNextMove(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime) {
    clock_t deadline = getDeadline(allocatedTime);
    while (hasTimeRemaining(deadline)) {
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
    return getMostSimulatedChildSquare(root);
}
// END FIND_NEXT_MOVE

















// START HANDLE_TURN
MCTSNode* handleEnemyTurn(Board* board, MCTSNode* root, Square enemyMove) {
    if (enemyMove.board == 9 && enemyMove.position == 9) {
        return root;
    }
    MCTSNode* newRoot = updateRoot(root, board, enemyMove);
    makePermanentMove(board, enemyMove);
    return newRoot;
}


HandleTurnResult handleTurn(Board* board, MCTSNode* root, pcg32_random_t* rng, double allocatedTime, Square enemyMove) {
    root = handleEnemyTurn(board, root, enemyMove);
    Square move = findNextMove(board, root, rng, allocatedTime);
    MCTSNode* newRoot = updateRoot(root, board, move);
    makePermanentMove(board, move);
    HandleTurnResult result = {move, newRoot};
    return result;
}
// END HANDLE_TURN






















// START MAIN
void skipMovesInput(FILE* file) {
    int validActionCount;
    int amountMatched = fscanf(file, "%d", &validActionCount);  // NOLINT(cert-err34-c)
    assertMsg(amountMatched == 1, "Incorrect amount of arguments matched");
    for (int i = 0; i < validActionCount; i++) {
        int row;
        int col;
        amountMatched = fscanf(file, "%d%d", &row, &col);  // NOLINT(cert-err34-c)
        assertMsg(amountMatched == 2, "Incorrect amount of arguments matched");
    }
}


void printMove(MCTSNode* root, Square bestMove) {
    Square s = toGameNotation(bestMove);
    uint8_t x = s.board;
    uint8_t y = s.position;
    double winrate = getWinrate(root);
    int sims = getSims(root);
    printf("%d %d %.4f %d\n", x, y, winrate, sims);
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
        printMove(root, result.move);
    }
    freeMCTSTree(root);
    freeBoard(board);
}


#define TIME 0.0999
int main() {
    // runTests();
    playGame(stdin, TIME);
}
// END MAIN