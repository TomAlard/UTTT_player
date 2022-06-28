#include "player.h"
#include "util.h"


Player otherPlayer(Player player) {
    if (player == NONE || player == BOTH) {
        crash("Invalid Player Enum value passed to otherPlayer");
    }
    return player == PLAYER1? PLAYER2 : PLAYER1;
}