#include "game.h"    //includes the contents of another file here, like game states, modes etc.
#include "pokemon.h"  //includes Pokémon-related definitions and functions.(pokemon, poke assign random())
#include "board.h"   //includes the board-related definitions and functions.(board_init(g))
#include <stdio.h>

void game_init(Game *g) {    //initializes the game, Game is a structure, *g pointer to a structure,
    g->state = STATE_MENU;   // boot to title screen (was: jump straight into gameplay),Access the state member of the Game
    g->currentPlayer = 0;    // player 0 goes first
    g->turnCount = 0;        // no turns played yet
    g->playerCount = 0;      // set later in the menu (was hardcoded to 2)
    g->mode = MODE_CLASSIC;  // default mode; menu lets the player pick Classic or Ladder

const char *names[] = {"Red", "Blue", "Yellow", "Green"};    //creates an array of pointers to constant characters.(names[0] gives "Red")
Color colors[] = {RED, BLUE, YELLOW, GREEN};                 //The array contains the colors corresponding to the players,Color is a raylib type
    for (int i = 0; i < MAX_PLAYERS; i++) {
        g->players[i].id = i;                   //g->players is the players array,[i] selects the current player,.id accesses that player's ID.
        g->players[i].name = names[i];          //gives each player their corresponding name.
        g->players[i].color = colors[i];        //gives each player their corresponding color.
        g->players[i].position = 0;             //sets the player's initial position to 0.
        g->players[i].finished = false;         //sets whether the player has finished the game, initially false, false is a boolean value
        g->players[i].finishOrder = 0;          //initializes the player's finishing position.
        g->players[i].wins = 0;                 //initializes the player's number of wins to zero.
        g->players[i].finishedCount = 0;        //the player currently has zero finished Pokémon/tokens,As tokens reach the destination, this value can increase
        g->players[i].pokemon = (Pokemon){0};   //Pokemon is a structure defined in Pokémon code,and initializes its fields to default zero values
        for (int k = 0; k < TOKENS_PER_PLAYER; k++) {
            g->players[i].tokens[k].owner = i;               //tells the token who owns it
            g->players[i].tokens[k].pokemon = (Pokemon){0};  //every token starts with a default Pokémon
            g->players[i].tokens[k].state = TOKEN_BASE;      //Every token starts inside its player's base
            g->players[i].tokens[k].progress = 0;            //the token has made zero progress along the board 
        }
    }

    poke_assign_random(g->players, g->playerCount);    //calls the function,Take the players currently in the game and randomly assign Pokémon to them
    board_init(g);                                     //calls the board initialization function 
}

void game_reset(Game *g) {      //to reset the game
    int count = g->playerCount; //saves the current number of players into a temporary variable called count
    GameMode mode = g->mode;    //saves the current game mode.
    game_init(g);               //essentially resets the game's data back to its initial state.
    g->playerCount = count;     //we restore the previous number of players
    g->mode = mode;                
    g->state = STATE_MENU;      //After resetting, the game goes back to the menu.
}

// Process one dice roll in battle
void battle_roll(Game *g) {
    // Battles are no longer limited to a fixed number of dice rolls.
    // They continue until one Pokemon reaches 0 HP.
    int roll = GetRandomValue(1, 6);
    g->battle.currentRoll = roll;
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

    // Check if battle is over: HP reaching 0 is the only end condition.
    if (g->battle.defenderHp <= 0 || g->battle.attackerHp <= 0) {
        g->battle.finished = true;
        if (g->battle.defenderHp <= 0 && g->battle.attackerHp > 0) {
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
