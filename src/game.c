#include "game.h"
#include "pokemon.h"
#include "board.h"
#include <stdio.h>

void game_init(Game *g) {
    g->state = STATE_MENU;   // boot to title screen (was: jump straight into gameplay)
    g->currentPlayer = 0;    // player 0 goes first
    g->turnCount = 0;        // no turns played yet
    g->playerCount = 0;      // set later in the menu (was hardcoded to 2)
    g->mode = MODE_CLASSIC;  // default mode; menu lets the player pick Classic or Ladder

    const char *names[] = {"Red", "Blue", "Green", "Yellow"};
    Color colors[] = {RED, BLUE, GREEN, YELLOW};

    for (int i = 0; i < MAX_PLAYERS; i++) {
        g->players[i].id = i;
        g->players[i].name = names[i];
        g->players[i].color = colors[i];
        g->players[i].position = 0;
        g->players[i].finished = false;
        g->players[i].finishOrder = 0;
        g->players[i].wins = 0;
        g->players[i].finishedCount = 0;
        g->players[i].pokemon = (Pokemon){0};
        for (int k = 0; k < TOKENS_PER_PLAYER; k++) {
            g->players[i].tokens[k].owner = i;
            g->players[i].tokens[k].pokemon = (Pokemon){0};
            g->players[i].tokens[k].state = TOKEN_BASE;
            g->players[i].tokens[k].progress = 0;
        }
    }

    poke_assign_random(g->players, g->playerCount);
    board_init(g);
}

void game_reset(Game *g) {
    int count = g->playerCount;
    GameMode mode = g->mode;
    game_init(g);
    g->playerCount = count;
    g->mode = mode;
    g->state = STATE_MENU;
}

// Process one dice roll in battle
void battle_roll(Game *g) {
    if (g->battle.rollsLeft <= 0) return;

    int roll = GetRandomValue(1, 6);
    int atkType, defType;
    if (g->mode == MODE_CLASSIC) {
        atkType = (int)g->players[g->battle.attackerIdx].tokens[g->battle.attackerToken].pokemon.type;
        defType = (int)g->players[g->battle.defenderIdx].tokens[g->battle.defenderToken].pokemon.type;
    } else {
        atkType = (int)g->players[g->battle.attackerIdx].pokemon.type;
        defType = (int)g->players[g->battle.defenderIdx].pokemon.type;
    }

    int atkBonus = (roll == atkType) ? TYPE_ADVANTAGE_BONUS : 0;
    int defBonus = (roll == defType) ? TYPE_ADVANTAGE_BONUS : 0;

    // Apply damage
    g->battle.defenderHp -= (5 + atkBonus * 3);
    g->battle.attackerHp -= (5 + defBonus * 3);

    // Clamp
    if (g->battle.defenderHp < 0) g->battle.defenderHp = 0;
    if (g->battle.attackerHp < 0) g->battle.attackerHp = 0;

    // Message
    if (atkBonus > 0 && defBonus > 0) {
        sprintf(g->battle.message, "Roll %d: Both land a hit!", roll);
    } else if (atkBonus > 0) {
        sprintf(g->battle.message, "Roll %d: Super effective!", roll);
    } else if (defBonus > 0) {
        sprintf(g->battle.message, "Roll %d: Counter attack!", roll);
    } else {
        sprintf(g->battle.message, "Roll %d: Both miss...", roll);
    }
    g->battle.messageTimer = 60;
    g->battle.currentRoll = roll;
    g->battle.rollsLeft--;

    // Check if battle is over
    if (g->battle.rollsLeft <= 0 || g->battle.defenderHp <= 0 || g->battle.attackerHp <= 0) {
        g->battle.finished = true;
        if (g->battle.attackerHp > g->battle.defenderHp) {
            g->battle.attackerWon = true;
            sprintf(g->battle.message, "%s wins the battle!",
                    g->players[g->battle.attackerIdx].name);
        } else {
            g->battle.attackerWon = false;
            sprintf(g->battle.message, "%s wins the battle!",
                    g->players[g->battle.defenderIdx].name);
        }
    }
}

// Update player HP after battle
void battle_end(Game *g) {
    int atk = g->battle.attackerIdx;
    int def = g->battle.defenderIdx;

    g->players[atk].pokemon.hp = g->battle.attackerHp;
    g->players[def].pokemon.hp = g->battle.defenderHp;

    if (g->battle.attackerWon) {
        // Defender goes back to start
        g->players[def].position = 0;
        g->players[atk].wins++;
    } else {
        // Attacker goes back to start
        g->players[atk].position = 0;
        g->players[def].wins++;
    }

    g->state = STATE_PLAYING;
}

// Main game update
void game_update(Game *g) {
    if (g->state == STATE_BATTLE) {
        if (g->battle.messageTimer > 0) g->battle.messageTimer--;
        return; // wait for space press to advance
    }
    if (g->state == STATE_BATTLE_RESULT) {
        if (IsKeyPressed(KEY_SPACE)) {
            battle_end(g);
            // Advance to next player
            g->currentPlayer = (g->currentPlayer + 1) % g->playerCount;
        }
        return;
    }
}
