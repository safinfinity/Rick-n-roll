#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

// ── Constants ──
#define WINDOW_W 1200
#define WINDOW_H 800
#define BOARD_SIZE 52            // Classic mode: 52-cell Ludo cross track (wraps 52 -> 1)
#define BOARD_SQUARES 30         // Ladder mode: single-token linear board
#define MAX_PLAYERS 4
#define TOKENS_PER_PLAYER 4      // Classic mode: 4 Pokemon tokens per player
#define POKEMON_POOL_SIZE 8
#define SHARED_TRACK_STEPS 52    // steps on shared cross track before home lane
#define HOME_STEPS 6             // steps through the private home lane
#define TOTAL_TRAVEL_STEPS 58    // SHARED_TRACK_STEPS + HOME_STEPS
#define MAX_POKEMON_PARTY 3 // Max pokemon per player (unused yet — planned for Ladder Mode)
#define DICE_ROLLS_PER_BATTLE 3
#define BASE_HP 100
#define TYPE_ADVANTAGE_BONUS 2
#define MAX_LOG 10 // Planned: circular log of events (unused yet)

// ── Pokemon Types ──
typedef enum {
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
    STATE_MODE_SELECT, 
    STATE_PLAYER_COUNT,
    STATE_DRAFT,
    STATE_PLAYING,// in use
    STATE_ROLLING,
    STATE_MOVING,
    STATE_BATTLE, // in in use
    STATE_BATTLE_RESULT,
    STATE_GAME_OVER // in use
} GameState;

// ── Square Types ── not implemented yet
typedef enum {
    SQ_NORMAL,
    SQ_SAFE,
    SQ_LADDER,
    SQ_SNAKE,
    SQ_EVOLUTION,
    SQ_HABITAT,
    SQ_MYSTERY,
    SQ_STONE
} SquareType;

// ── Pokemon ──
typedef struct {
    const char *name;
    PokeType type;
    int hp;
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
    TokenState state;
    int progress;       // 0 = base; 1..52 shared track; 53..57 home lane; 58 = goal
} Token;

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
    int position;
    Pokemon pokemon; // Ladder mode bridge (single token)
    int wins;
    bool finished;
    int finishOrder;
    int finishedCount;
    Token tokens[TOKENS_PER_PLAYER]; // Classic mode
} Player;

// ── Board Square ──
/*id        → display number (1-52)
type      → SQ_NORMAL, SQ_SAFE, etc.
screenPos → Vector2 {x, y} pixel center of this square on screen
*/
typedef struct {
    int id;
    SquareType type;
    Vector2 screenPos;
} BoardSquare;

// ── Dice ──
typedef struct {
    int value;
    bool rolling;
    int rollTimer;//frames elapsed since roll started
    int rollDuration;//total frames for animation (set to 30 = 0.5 sec at 60fps)
} Dice;

// ── Battle State ──
typedef struct {
    int attackerIdx;      // player index of the attacker
    int defenderIdx;      // player index of the defender
    int attackerToken;    // token index (Classic Mode)
    int defenderToken;    // token index (Classic Mode)
    int rollsLeft;
    int currentRoll;
    int attackerHp;
    int defenderHp;
    int attackerMaxHp;
    int defenderMaxHp;
    char message[128];
    int messageTimer;
    int flashTimer;
    bool finished;
    bool attackerWon;
} BattleState;

// ── Main Game State ──
typedef struct {
    GameState state;
    GameMode mode;
    int playerCount;
    Player players[MAX_PLAYERS];
    int currentPlayer;
    BoardSquare board[BOARD_SIZE];
    Vector2 homeLanePos[MAX_PLAYERS][HOME_STEPS];
    Vector2 basePos[MAX_PLAYERS][TOKENS_PER_PLAYER];
    Dice dice;
    BattleState battle;
    int turnCount;
    Texture2D pokeSprites[9]; // indexed by PokeType (1-6)
} Game;

// ── Function declarations ──
void game_init(Game *g);
void game_reset(Game *g);
const char* poke_type_name(PokeType t);
Color poke_type_color(PokeType t);
bool poke_type_advantage(PokeType a, PokeType b);
void board_init(Game *g);
Vector2 board_get_pos(BoardSquare *sq);
void dice_roll(Dice *d);
void dice_update(Dice *d);
void dice_draw(Dice *d, int x, int y);
void battle_draw(Game *g);
void menu_init(void);
int menu_update(Game *g);
void menu_draw(Game *g);
void board_draw(Game *g);
void game_update(Game *g);
void battle_roll(Game *g);
void battle_end(Game *g);

#endif // GAME_H
