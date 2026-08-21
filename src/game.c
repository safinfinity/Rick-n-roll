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

const char *names[] = {"Red", "Blue", "Yellow", "Green"};
Color colors[] = {RED, BLUE, YELLOW, GREEN};
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
// Check actual Pokemon type advantages
bool atkAdvantage = poke_type_advantage(
    (PokeType)atkType,
    (PokeType)defType
);

bool defAdvantage = poke_type_advantage(
    (PokeType)defType,
    (PokeType)atkType
);

// Get the actual Pokemon stats
int atkStat;
int defStat;
int defenderDef;
int attackerDef;

if (g->mode == MODE_CLASSIC) {
    Pokemon *atkPokemon =
        &g->players[g->battle.attackerIdx]
             .tokens[g->battle.attackerToken].pokemon;

    Pokemon *defPokemon =
        &g->players[g->battle.defenderIdx]
             .tokens[g->battle.defenderToken].pokemon;

    atkStat = atkPokemon->atk;
    defStat = defPokemon->atk;

    defenderDef = defPokemon->def;
    attackerDef = atkPokemon->def;

} else {

    atkStat = g->players[g->battle.attackerIdx].pokemon.atk;
    defStat = g->players[g->battle.defenderIdx].pokemon.atk;

    defenderDef = g->players[g->battle.defenderIdx].pokemon.def;
    attackerDef = g->players[g->battle.attackerIdx].pokemon.def;
}

// Base damage comes from ATK
int atkDamage = atkStat;
int defDamage = defStat;

// Type advantage = 50% more damage
if (atkAdvantage) {
    atkDamage = (atkDamage * 3) / 2;
}

if (defAdvantage) {
    defDamage = (defDamage * 3) / 2;
}

// Rolling 6 = critical hit
if (roll == 6) {
    atkDamage *= 2;
    defDamage *= 2;
}

// DEF reduces incoming damage
atkDamage -= defenderDef;
defDamage -= attackerDef;

// Always deal at least 1 damage
if (atkDamage < 1)
    atkDamage = 1;

if (defDamage < 1)
    defDamage = 1;

// Apply damage
g->battle.defenderHp -= atkDamage;
g->battle.attackerHp -= defDamage;
    // Clamp
    if (g->battle.defenderHp < 0) g->battle.defenderHp = 0;
    if (g->battle.attackerHp < 0) g->battle.attackerHp = 0;

// Battle message
if (atkAdvantage && defAdvantage) {
    sprintf(g->battle.message,
            "Roll %d: Both have type advantage!",
            roll);

} else if (atkAdvantage && roll == 6) {
    sprintf(g->battle.message,
            "CRITICAL! Type advantage!");

} else if (defAdvantage && roll == 6) {
    sprintf(g->battle.message,
            "CRITICAL! Defender strikes back!");

} else if (atkAdvantage) {
    sprintf(g->battle.message,
            "Roll %d: Super effective!",
            roll);

} else if (defAdvantage) {
    sprintf(g->battle.message,
            "Roll %d: Defender has type advantage!",
            roll);

} else if (roll == 6) {
    sprintf(g->battle.message,
            "Roll %d: CRITICAL HIT!",
            roll);

} else {
    sprintf(g->battle.message,
            "Roll %d: Normal attack!",
            roll);
}

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
