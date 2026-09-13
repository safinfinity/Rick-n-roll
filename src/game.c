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
void battle_roll(Game *g) {    //handles one dice roll during a Pokémon battle
    // Battles are no longer limited to a fixed number of dice rolls.
    // They continue until one Pokemon reaches 0 HP.
    int roll = GetRandomValue(1, 6);   //raylib function
    g->battle.currentRoll = roll;      //Take the dice number we just generated and save it as the current battle roll.
    int atkType, defType;              //Will store the attacker and defender Pokémon's type            
    if (g->mode == MODE_CLASSIC) {     
        atkType = (int)g->players[g->battle.attackerIdx].tokens[g->battle.attackerToken].pokemon.type;     //Find the attacker's Pokémon and get its type,since player gives an enum value,its converted to int
        defType = (int)g->players[g->battle.defenderIdx].tokens[g->battle.defenderToken].pokemon.type;     //Find the defender's Pokémon and get its type,since player gives an enum value,its converted to int
    } else {
        atkType = (int)g->players[g->battle.attackerIdx].pokemon.type;      //finds the attacking player,its pokemon and its type,and converts it to an integer
        defType = (int)g->players[g->battle.defenderIdx].pokemon.type;
    }
// Check actual Pokemon type advantages
bool atkAdvantage = poke_type_advantage(      //Does the attacker's type have an advantage over the defender's type?
    (PokeType)atkType,               //coverts from int to poketype
    (PokeType)defType
);

bool defAdvantage = poke_type_advantage(
    (PokeType)defType,
    (PokeType)atkType
);

// Get the actual Pokemon stats
int atkStat;             //stores Attacker's ATK stat.
int defStat;             //stores Defenders atk stat.
int defenderDef;         //Stores the defender Pokémon's DEF stat.
int attackerDef;

if (g->mode == MODE_CLASSIC) {
    Pokemon *atkPokemon =
        &g->players[g->battle.attackerIdx]
             .tokens[g->battle.attackerToken].pokemon; //pointer to a Pokemon structure;Find the attacker's Pokémon and store its address in atkPokemon.

    Pokemon *defPokemon =
        &g->players[g->battle.defenderIdx]
             .tokens[g->battle.defenderToken].pokemon;   //atkPokemon and defPokemon point to the actual Pokémon stored in the game.

    atkStat = atkPokemon->atk;      //Access the atk member of the Pokémon pointed to by atkPokemon.
    defStat = defPokemon->atk;

    defenderDef = defPokemon->def;
    attackerDef = atkPokemon->def;

} else {

    atkStat = g->players[g->battle.attackerIdx].pokemon.atk;   //attacking player,their pokemon,atk and stored in atk stat
    defStat = g->players[g->battle.defenderIdx].pokemon.atk;

    defenderDef = g->players[g->battle.defenderIdx].pokemon.def;
    attackerDef = g->players[g->battle.attackerIdx].pokemon.def;
}

// Base damage comes from ATK
int atkDamage = atkStat;
int defDamage = defStat;

// Type advantage = 50% more damage
if (atkAdvantage) {
    atkDamage = (atkDamage * 3) / 2;      //if attacker has type advantage, increase damage by 50%(1.5 times)
}

if (defAdvantage) {
    defDamage = (defDamage * 3) / 2;
}

// Rolling 6 = critical hit
if (roll == 6) {
    atkDamage *= 2;   //damage is doubled,regardless of a type advantage
    defDamage *= 2;
}

// DEF reduces incoming damage
atkDamage -= defenderDef;     //attackers atk damage-defenders def damage=final damage
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
    if (g->battle.defenderHp < 0) g->battle.defenderHp = 0;        //Don't allow HP to go below 0.
    if (g->battle.attackerHp < 0) g->battle.attackerHp = 0;

// Battle message
if (atkAdvantage && defAdvantage) {
    sprintf(g->battle.message,
            "Roll %d: Both have type advantage!",
            roll);      //sprintf → build a sentence → put it into message;

} else if (atkAdvantage && roll == 6) {
    sprintf(g->battle.message,
            "CRITICAL! Type advantage!");

} else if (defAdvantage && roll == 6) {
    sprintf(g->battle.message,
            "CRITICAL! Defender strikes back!");     //The defender had the advantage and got a critical hit

} else if (atkAdvantage) {
    sprintf(g->battle.message,
            "Roll %d: Super effective!",
            roll);    //"Super effective" means the attacker's Pokémon has a type advantage.

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
            g->battle.attackerWon = true;                     //the defender lost while the attacker survived.
            sprintf(g->battle.message, "%s wins the battle!",
                    g->players[g->battle.attackerIdx].name);
        } else {
            g->battle.attackerWon = false;
            sprintf(g->battle.message, "%s wins the battle!",
                    g->players[g->battle.defenderIdx].name);  //The attacker did not win.
        }
    }
}

// Update player HP after battle
void battle_end(Game *g) {
    int atk = g->battle.attackerIdx;    //creates an integer and stores attacker players idex in it
    int def = g->battle.defenderIdx;    

    g->players[atk].pokemon.hp = g->battle.attackerHp;  //takes the HP stored in the battle and saves it back into the player's Pokémon
    g->players[def].pokemon.hp = g->battle.defenderHp;

    if (g->battle.attackerWon) {
        // Defender goes back to start
        g->players[def].position = 0;      //the defender's player/token is sent back to the starting/base position
        g->players[atk].wins++;            //++ means increase win by 1
    } else {
        // Attacker goes back to start
        g->players[atk].position = 0;    //Send the attacker back to position 0
        g->players[def].wins++;
    }

    g->state = STATE_PLAYING;     //The battle is finished, so return to the normal Ludo gameplay
}

// Main game update
void game_update(Game *g) {            //to update the game depending on its current state
    if (g->state == STATE_BATTLE) {
        if (g->battle.messageTimer > 0) g->battle.messageTimer--;    //used to control how long the battle message stays active
        return; // wait for space press to advance
    }
    if (g->state == STATE_BATTLE_RESULT) {
        if (IsKeyPressed(KEY_SPACE)) {     //a raylib function,checks whether a particular keyboard key was pressed
            battle_end(g);
            // Advance to next player
            g->currentPlayer = (g->currentPlayer + 1) % g->playerCount;      //After the battle, give the turn to the next player
        }
        return;
    }
}
