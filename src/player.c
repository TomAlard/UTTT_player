#include "player.h"


Player otherPlayer(Player player) {
    return player == PLAYER1? PLAYER2 : PLAYER1;
}


bool playerIsWinner(Player player, Winner winner) {
    return (player == PLAYER1 && winner == WIN_P1) || (player == PLAYER2 && winner == WIN_P2);
}