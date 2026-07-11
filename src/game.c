#include "game.h"
#include "board.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

void game_init(Game *g) {
    srand((unsigned int)time(NULL));
    memset(g, 0, sizeof(Game));

    const char* names[] = {"Red", "Blue", "Green", "Yellow"};
    Color colors[] = {RED, BLUE, GREEN, YELLOW};

    for (int i = 0; i < MAX_PLAYERS; i++) {
        g->players[i].id = i;
        g->players[i].name = names[i];
        g->players[i].color = colors[i];
        g->players[i].position = 0;
        g->players[i].pokemon = (Pokemon){0};
        g->players[i].wins = 0;
        g->players[i].finished = false;
        g->players[i].finishOrder = 0;
    }

    g->state = STATE_MENU;
    g->mode = MODE_CLASSIC;
    g->playerCount = 2;
    g->currentPlayer = 0;
    g->dice.value = 0;
    g->dice.rolling = false;
    g->dice.rollTimer = 0;
    g->dice.rollDuration = 15;
    g->turnCount = 0;

    board_init(g);
}

void game_reset(Game *g) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        g->players[i].position = 0;
        g->players[i].pokemon = (Pokemon){0};
        g->players[i].finished = false;
        g->players[i].finishOrder = 0;
    }
    g->currentPlayer = 0;
    g->dice.value = 0;
    g->dice.rolling = false;
    g->dice.rollTimer = 0;
    g->turnCount = 0;
    g->state = STATE_MODE_SELECT;
    board_init(g);
}
