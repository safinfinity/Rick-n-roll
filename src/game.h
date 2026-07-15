#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

// ── Constants ──
#define WINDOW_W 1200
#define WINDOW_H 800
#define BOARD_SQUARES 30
#define MAX_PLAYERS 4
#define MAX_POKEMON_PARTY 3 // Max pokemon per player (unused yet — planned for Ladder Mode)
#define DICE_ROLLS_PER_BATTLE 3
#define BASE_HP 100
#define TYPE_ADVANTAGE_BONUS 2
#define MAX_LOG 10 // Planned: circular log of events (unused yet)

// ── Pokemon Types ──
typedef enum {
    POKE_NONE = 0, // enum assigns name for numbers
    POKE_FIRE = 1,
    POKE_WATER = 2,
    POKE_GRASS = 3,
    POKE_ELECTRIC = 4,
    POKE_PSYCHIC = 5,
    POKE_DRAGON = 6
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

// ── Player ──
// id          → 0-3 index
// name        → "Red", "Blue", "Green", "Yellow"
// color       → RED, BLUE, GREEN, YELLOW
// position    → 0 = start, 1-29 = on board, 30 = HOME
// pokemon     → ONE Pokemon struct embedded directly (not a pointer — the Pokemon lives INSIDE the Player)
// wins        → count of battles won
// finished    → true when reached BOARD_SQUARES
// finishOrder → 1 = first to finish, 2 = second, etc.
    typedef struct {
    int id;
    const char *name;
    Color color;
    int position;
    Pokemon pokemon; // struct within struct
    int wins;
    bool finished;
    int finishOrder;
} Player;

// ── Board Square ──
/*id        → display number (1-30)
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
    int attackerIdx;
    int defenderIdx;
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
    BoardSquare board[BOARD_SQUARES];
    Dice dice;
    BattleState battle;
    int turnCount;
    Texture2D pokeSprites[7]; // indexed by PokeType (1-6)
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
