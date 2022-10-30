#include "handle_turn.h"
#include "misc/util.h"


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


HandleTurnResult handleTurn(Board* board, MCTSNode* root, RNG* rng, double allocatedTime, Square enemyMove, int gameId) {
    root = handleEnemyTurn(board, root, enemyMove, rng);
    Square openingMove = {9, 9};
    if (openingMove.board != 9) {
        MCTSNode* newRoot = updateRoot(root, board, openingMove);
        makePermanentMove(board, openingMove);
        HandleTurnResult result = {openingMove, newRoot, 0};
        return result;
    }
    int amountOfSimulations = findNextMove(board, root, rng, allocatedTime, gameId);
    Square move = getMostPromisingMove(root);
    MCTSNode* newRoot = updateRoot(root, board, move);
    makePermanentMove(board, move);
    HandleTurnResult result = {move, newRoot, amountOfSimulations};
    return result;
}