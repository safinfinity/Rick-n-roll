#ifndef GAME_H    //#ifndef means "if not defined",prevents the header file from being included more than once
#define GAME_H    //If GAME_H hasn't been defined, this line defines it
#define DEBUG_DICE 1

#include "raylib.h"      //includes Raylib's header file for its types and functions
#include <stdbool.h>     //To use Boolean values such as bool, true, and false.

// ── Constants ──
#define WINDOW_W 1200            //game window is intended to be 1200 pixels wide.
#define WINDOW_H 800             //the window becomes 800 pixels high wherever this constant is used.
#define BOARD_SIZE 52            // Classic mode: 52-cell Ludo cross track (wraps 52 -> 1)
#define BOARD_SQUARES 30         // Ladder mode: single-token linear board
#define MAX_PLAYERS 4
#define TOKENS_PER_PLAYER 4      // Classic mode: 4 Pokemon tokens per player
#define POKEMON_POOL_SIZE 8      //the Pokémon pool contains 8 Pokémon.
#define SHARED_TRACK_STEPS 52    // steps on shared cross track before home lane
#define HOME_STEPS 6             // steps through the private home lane
#define TOTAL_TRAVEL_STEPS 58    // SHARED_TRACK_STEPS + HOME_STEPS
#define MAX_POKEMON_PARTY 3 // Max pokemon per player (unused yet — planned for Ladder Mode)
#define DICE_ROLLS_PER_BATTLE 0 // 0 = unlimited; battles end only when a Pokemon reaches 0 HP
#define BASE_HP 100             //This sets the base Pokémon HP to 100
#define TYPE_ADVANTAGE_BONUS 2  //defines the value associated with the type advantage bonus.
#define MAX_LOG 10 // Planned: circular log of events (unused yet)

// ── Pokemon Types ──
typedef enum {           //An enum allows us to give meaningful names to integer values.
    POKE_NONE = 0,
    POKE_FIRE = 1,
    POKE_WATER = 2,
    POKE_GRASS = 3,
    POKE_ELECTRIC = 4,
    POKE_PSYCHIC = 5,
    POKE_DRAGON = 6,
    POKE_ICE = 7,
    POKE_FIGHTING = 8
} PokeType;

// ── Game Modes ──
typedef enum {
    MODE_CLASSIC,//0
    MODE_LADDER //1
} GameMode;

// ── Game States ──
typedef enum {
    STATE_MENU, //in use
    STATE_MODE_SELECT,          //Used for selecting Classic or Ladder mode.
    STATE_PLAYER_COUNT,         //Used for selecting the number of players.
    //STATE_DRAFT,
    STATE_PLAYING,// in use,Its automatic value is 3,assuming the preceding three states are active.actual game board is running
    STATE_ROLLING,//a state where the player is rolling the dice.
    STATE_MOVING, //a state where a token/player is moving.
    STATE_BATTLE, // in in use,represents an active Pokémon battle.
    STATE_BATTLE_RESULT, //represents the state after the battle has finished, where the result is shown.
    STATE_GAME_OVER // in use
} GameState;

// ── Square Types ── not implemented yet
typedef enum {
    SQ_NORMAL,   //normal sq=0
    SQ_SAFE,     //safe sq=1
    SQ_LADDER,   //ladder sq=2
    SQ_SNAKE,
    SQ_EVOLUTION,
    SQ_HABITAT,
    SQ_MYSTERY,
    SQ_STONE
} SquareType;

// ── Pokemon ──
typedef struct {
    const char *name;    //name points to a string whose characters we don't intend to change
    PokeType type;       //PokeType is the enum we created to represent Pokémon types
    int hp;              //stores the Pokémon's current HP
    int maxHp;
    int atk;// atk, def not used yet
    int def;
    Color color;//color → raylib Color for drawing (set from type_colors[])
} Pokemon;

// ── Token (Classic Mode) ──
// One Pokemon piece on the Ludo board. Each player owns TOKENS_PER_PLAYER.
typedef enum {
    TOKEN_BASE = 0,     // in the base yard, needs a roll of 6 to deploy
    TOKEN_ACTIVE,       // on the shared 52-cell cross track (progress 1..52)
    TOKEN_HOME,         // inside its owner's private home lane (progress 53..57)
    TOKEN_FINISHED      // reached the final goal (progress 58)
} TokenState;

typedef struct {
    int owner;          // owning player id (0-3)
    Pokemon pokemon;    // the Pokemon identity of this token
    TokenState state;   //tells us where the token currently is
    int progress;       // 0 = base; 1..52 shared track; 53..57 home lane; 58 = goal
} Token;                //progress Tells you exactly where the token is,state tells you the category

// ── Player ──
// id          → 0-3 index
// name        → "Red", "Blue", "Green", "Yellow"
// color       → RED, BLUE, GREEN, YELLOW
// position    → 0 = start, 1-29 = on board, 30 = HOME (Ladder Mode single token)
// pokemon     → Ladder Mode: ONE Pokemon struct embedded directly (not a pointer)
// tokens      → Classic Mode: TOKENS_PER_PLAYER tokens, each with own Pokemon
// wins        → count of battles won
// finished    → true when the player finished (Classic: all tokens home)
// finishOrder → 1 = first to finish, 2 = second, etc.
// finishedCount → Classic Mode: how many of this player's tokens reached the goal
    typedef struct {
    int id;
    const char *name;
    Color color;
    int position;    //stores the player's/current token-like position used by your Ladder mode bridge
    Pokemon pokemon; // Ladder mode bridge (single token)
    int wins;
    bool finished;   //A Boolean value indicating whether the player has finished the game
    int finishOrder; //Stores the order in which players finish
    int finishedCount; //keeps track of how many relevant things have finished
    Token tokens[TOKENS_PER_PLAYER]; // Classic mode
} Player;

// ── Board Square ──
/*id        → display number (1-52)
type      → SQ_NORMAL, SQ_SAFE, etc.
screenPos → Vector2 {x, y} pixel center of this square on screen
*/
typedef struct {      //a structure representing one square/cell of the board.
    int id;           //Stores the square's number
    SquareType type;
    Vector2 screenPos;//Vector2 is a Raylib type representing a 2D coordinate;the center of that square is located at pixel coordinate (x,y) on the screen.
} BoardSquare;

// ── Dice ──
typedef struct {  //Creates a structure representing the game's dice
    int value;    //Stores the current dice value
    bool rolling; //whether the dice is currently being rolled
    int rollTimer;//frames elapsed since roll started
    int rollDuration;//total frames for animation (set to 30 = 0.5 sec at 60fps)
} Dice;

// ── Battle State ──
typedef struct {          //Creates a structure containing all the information needed during a Pokémon battle
    int attackerIdx;      // player index of the attacker
    int defenderIdx;      // player index of the defender
    int attackerToken;    // token index (Classic Mode)
    int defenderToken;    // token index (Classic Mode)
    int rollsLeft;        //Stores how many battle rolls remain
    int currentRoll;      //Stores the most recent dice roll
    int attackerHp;       //Stores the attacker's current HP during the battle
    int defenderHp;       //Stores the defender's current HP during the battle
    int attackerMaxHp;    //store the maximum HP values for the attacker
    int defenderMaxHp;    //store the maximum HP values for the defender
    char message[128];
    int messageTimer;     //Stores how long the battle message should remain active
    int flashTimer;       //Likely used for a visual flashing effect during battle
    bool finished;        //Indicates whether the battle has ended
    bool attackerWon;
} BattleState;

// ── Main Game State ──
typedef struct {       //creates the structure that represents the whole game
    GameState state;   //what the game is currently doing
    GameMode mode;
    int playerCount;   //how many players are actually playing
    Player players[MAX_PLAYERS];
    int currentPlayer;
    BoardSquare board[BOARD_SIZE];//Creates an array of board squares
    Vector2 homeLanePos[MAX_PLAYERS][HOME_STEPS];//where to draw each player's home-lane squares on the screen
    Vector2 basePos[MAX_PLAYERS][TOKENS_PER_PLAYER];//stores the screen coordinates for each player's tokens while they are in the base
    Dice dice;
    BattleState battle;
    int turnCount;//Stores how many turns have occurred
    Texture2D pokeSprites[9]; //indexed by PokeType (1-8)
    Texture2D pokeballTexture; // indexed by PokeType (1-6)
} Game;

// ── Function declarations ──
void game_init(Game *g);
void game_reset(Game *g);
const char* poke_type_name(PokeType t);
Color poke_type_color(PokeType t);//Takes a Pokémon type and returns a Raylib Color
bool poke_type_advantage(PokeType a, PokeType b);//answers:Does type a have an advantage over type b
void board_init(Game *g);
Vector2 board_get_pos(BoardSquare *sq);//Takes a pointer to a board square and returns its screen position
void dice_roll(Dice *d);  //Rolls/starts the dice
void dice_update(Dice *d);//Updates dice animation/state
void dice_draw(Dice *d, int x, int y);//Draws the dice at screen coordinates (x,y)
void battle_draw(Game *g);//Draws the battle screen
void menu_init(void);//void inside the brackets means:the function takes no arguments
int menu_update(Game *g);//Updates the menu and returns an integer
void menu_draw(Game *g);
void board_draw(Game *g);
void game_update(Game *g);//Updates the game's logic based on the current state
void battle_roll(Game *g);//Performs one battle roll and calculates damage
void battle_end(Game *g);//Finishes the battle and updates the game/player information

#endif // GAME_H
