#include "player.h"


Player otherPlayer(Player player) {
    return player == PLAYER1? PLAYER2 : PLAYER1;
}