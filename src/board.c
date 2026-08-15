#include "game.h"
#include "board.h"
#include <string.h>
#include <math.h>

// ── Ladder board layout (single 30-square serpentine grid) ──
#define BOARD_COLS 6
#define BOARD_ROWS 5
#define CELL_SIZE 100
#define BOARD_X 60
#define BOARD_Y 140

// ── Classic board layout (52-cell Ludo cross on a 15x15 grid) ──
#define LUDO_GRID 15
#define LUDO_CELL 44.0f
#define LUDO_X 30.0f
#define LUDO_Y 100.0f

// 52-cell common path on the 15x15 grid (from bocaletto-luca/Ludo).
// Cell (r,c) = row r, column c, 0-indexed; index 0 = Red's start, then
// 13 = Blue, 26 = Yellow, 39 = Green.
static const int path_r[BOARD_SIZE] = {
    6,6,6,6,6,6, 5,4,3,2,1,0, 0,0, 1,2,3,4,5, 6,6,6,6,6,6, 7,8, 8,8,8,8,8,
    9,10,11,12,13,14, 14,14, 13,12,11,10,9, 8,8,8,8,8,8, 7
};
static const int path_c[BOARD_SIZE] = {
    0,1,2,3,4,5, 6,6,6,6,6,6, 7,8, 8,8,8,8,8, 9,10,11,12,13,14, 14,14,
    13,12,11,10,9, 8,8,8,8,8,8, 7,6, 6,6,6,6,6, 5,4,3,2,1,0, 0
};

static Vector2 ludo_pos(int r, int c) {
    return (Vector2){ LUDO_X + (c + 0.5f) * LUDO_CELL, LUDO_Y + (r + 0.5f) * LUDO_CELL };
}

// ── Classic Mode geometry ──

int GetStartSquare(int player) {
    static const int starts[MAX_PLAYERS] = {1, 14, 40, 27}; // Red, Blue, Green, Yellow
    if (player < 0 || player >= MAX_PLAYERS) return 1;
    return starts[player];
}

int GetSharedBoardSquare(int player, int progress) {
    int start = GetStartSquare(player);
    return ((start - 1 + (progress - 1)) % BOARD_SIZE) + 1;
}

int GetNextBoardSquare(int currentSquare) {
    return (currentSquare % BOARD_SIZE) + 1;
}

bool IsSafeSquare(int square) {
    switch (square) {
        case 1: case 14: case 27: case 40: // starting squares (Red, Blue, Yellow, Green)
        case 7: case 20: case 33: case 46: // mid-sector star squares
            return true;
        default:
            return false;
    }
}

bool CanDeployToken(const Token *t, int dice) {
    return t->state == TOKEN_BASE && dice == 6;
}

bool CanMoveToken(const Token *t, int dice) {
    if (t->state == TOKEN_BASE || t->state == TOKEN_FINISHED) return false;
    return t->progress + dice <= TOTAL_TRAVEL_STEPS;
}

bool IsOnSharedTrack(const Token *t) { return t->state == TOKEN_ACTIVE; }
bool IsInHomeLane(const Token *t)     { return t->state == TOKEN_HOME; }

void MoveToken(Token *t, int dice) {
    t->progress += dice;
    if (t->progress >= TOTAL_TRAVEL_STEPS) {
        t->progress = TOTAL_TRAVEL_STEPS;
        t->state = TOKEN_FINISHED;
    } else if (t->progress > SHARED_TRACK_STEPS) {
        t->state = TOKEN_HOME;
    } else {
        t->state = TOKEN_ACTIVE;
    }
}

void SendTokenToBase(Token *t) {
    t->progress = 0;
    t->state = TOKEN_BASE;
}

// Home lanes run 6 cells from the shared-track entry straight into the center.
// Base yards hold the 2 token spots in the four corner quadrants.
static void classic_home_and_base(Game *g) {
    // Home lane grid coords per player (inward toward center (7,7)).
    static const int hr[MAX_PLAYERS][HOME_STEPS] = {
        {7, 7, 7, 7, 7, 7},       // Red:   row 7, cols 1..6 (enters from (7,0))
        {1, 2, 3, 4, 5, 6},       // Blue:  col 7, rows 1..6 (enters from (0,7))
        {13, 12, 11, 10, 9, 8},   // Green: col 7, rows 13..8 (enters from (14,7))
        {7, 7, 7, 7, 7, 7}        // Yellow: row 7, cols 13..8 (enters from (7,14))
    };
    static const int hc[MAX_PLAYERS][HOME_STEPS] = {
        {1, 2, 3, 4, 5, 6},
        {7, 7, 7, 7, 7, 7},
        {7, 7, 7, 7, 7, 7},
        {13, 12, 11, 10, 9, 8}
    };
    // Base token spots: 2 diagonal cells of each corner 2x2 yard.
    static const int br[MAX_PLAYERS][TOKENS_PER_PLAYER] = {
        {2, 4},    // Red (top-left yard)
        {2, 4},    // Blue (top-right yard)
        {10, 12},  // Green (bottom-right yard)
        {10, 12}   // Yellow (bottom-left yard)
    };
    static const int bc[MAX_PLAYERS][TOKENS_PER_PLAYER] = {
        {2, 4},    // Red cols 2,4
        {10, 12},  // Blue cols 10,12
        {10, 12},  // Green cols 10,12
        {2, 4}     // Yellow cols 2,4
    };

    for (int p = 0; p < MAX_PLAYERS; p++) {
        for (int i = 0; i < HOME_STEPS; i++) {
            g->homeLanePos[p][i] = ludo_pos(hr[p][i], hc[p][i]);
        }
        for (int k = 0; k < TOKENS_PER_PLAYER; k++) {
            g->basePos[p][k] = ludo_pos(br[p][k], bc[p][k]);
        }
    }
}

// ── Board init ──

void board_init(Game *g) {
    memset(g->board, 0, sizeof(g->board));

    if (g->mode == MODE_CLASSIC) {
        for (int i = 0; i < BOARD_SIZE; i++) {
            g->board[i].id = i + 1;
            g->board[i].type = IsSafeSquare(i + 1) ? SQ_SAFE : SQ_NORMAL;
            g->board[i].screenPos = ludo_pos(path_r[i], path_c[i]);
        }
        classic_home_and_base(g);
        return;
    }

    // Ladder mode: existing 30-square serpentine layout
    int sq = 1;
    for (int row = BOARD_ROWS - 1; row >= 0; row--) {
        if ((BOARD_ROWS - 1 - row) % 2 == 0) {
            for (int col = 0; col < BOARD_COLS; col++) {
                int idx = sq - 1;
                if (idx < BOARD_SQUARES) {
                    g->board[idx].id = sq;
                    g->board[idx].type = SQ_NORMAL;
                    g->board[idx].screenPos.x = BOARD_X + col * CELL_SIZE + CELL_SIZE / 2.0f;
                    g->board[idx].screenPos.y = BOARD_Y + row * CELL_SIZE + CELL_SIZE / 2.0f;
                }
                sq++;
            }
        } else {
            for (int col = BOARD_COLS - 1; col >= 0; col--) {
                int idx = sq - 1;
                if (idx < BOARD_SQUARES) {
                    g->board[idx].id = sq;
                    g->board[idx].type = SQ_NORMAL;
                    g->board[idx].screenPos.x = BOARD_X + col * CELL_SIZE + CELL_SIZE / 2.0f;
                    g->board[idx].screenPos.y = BOARD_Y + row * CELL_SIZE + CELL_SIZE / 2.0f;
                }
                sq++;
            }
        }
    }

    g->board[2].type  = SQ_LADDER;
    g->board[4].type  = SQ_SNAKE;
    g->board[7].type  = SQ_SAFE;
    g->board[9].type  = SQ_MYSTERY;
    g->board[11].type = SQ_LADDER;
    g->board[13].type = SQ_EVOLUTION;
    g->board[15].type = SQ_SNAKE;
    g->board[17].type = SQ_SAFE;
    g->board[19].type = SQ_HABITAT;
    g->board[21].type = SQ_MYSTERY;
    g->board[23].type = SQ_LADDER;
    g->board[25].type = SQ_STONE;
    g->board[27].type = SQ_HABITAT;
    g->board[29].type = SQ_SAFE;
}

Vector2 board_get_pos(BoardSquare *sq) {
    return sq->screenPos;
}

int board_find_square_type(Game *g, int position) {
    if (position < 1 || position > BOARD_SQUARES) return SQ_NORMAL;
    return g->board[position - 1].type;
}

int board_get_ladder_dest(int position) {
    switch (position) {
        case 3:  return 12;
        case 12: return 22;
        case 24: return 28;
        default: return -1;
    }
}

int board_get_snake_dest(int position) {
    switch (position) {
        case 5:  return 1;
        case 16: return 8;
        default: return -1;
    }
}

#ifdef TEST_CLASSIC
#include <assert.h>
#include <stdio.h>

int main(void) {
    // Start squares: Red, Blue, Green, Yellow
    assert(GetStartSquare(0) == 1);
    assert(GetStartSquare(1) == 14);
    assert(GetStartSquare(2) == 40);
    assert(GetStartSquare(3) == 27);

    // Wraparound
    assert(GetNextBoardSquare(52) == 1);
    assert(GetNextBoardSquare(1) == 2);

    // Player-specific shared squares: 52 steps = last shared square, no home yet
    assert(GetSharedBoardSquare(0, 1) == 1);
    assert(GetSharedBoardSquare(0, 52) == 52);
    assert(GetSharedBoardSquare(1, 1) == 14);
    assert(GetSharedBoardSquare(1, 52) == 13);
    assert(GetSharedBoardSquare(2, 1) == 40);
    assert(GetSharedBoardSquare(2, 52) == 39);
    assert(GetSharedBoardSquare(3, 1) == 27);
    assert(GetSharedBoardSquare(3, 52) == 26);

    // Safe squares: starts + mid-sector stars
    assert(IsSafeSquare(1) && IsSafeSquare(14) && IsSafeSquare(27) && IsSafeSquare(40));
    assert(IsSafeSquare(7) && IsSafeSquare(20) && IsSafeSquare(33) && IsSafeSquare(46));
    assert(!IsSafeSquare(2) && !IsSafeSquare(52) && !IsSafeSquare(30));

    // Deployment: only exact 6
    Token t = {0};
    t.state = TOKEN_BASE;
    assert(!CanDeployToken(&t, 5));
    assert(CanDeployToken(&t, 6));
    assert(!CanMoveToken(&t, 6)); // base tokens cannot move

    // Overshoot rejection
    t.state = TOKEN_ACTIVE;
    t.progress = 56;
    assert(!CanMoveToken(&t, 3)); // 56+3 > 58
    assert(CanMoveToken(&t, 2));  // 56+2 == 58
    MoveToken(&t, 2);
    assert(t.progress == 58 && t.state == TOKEN_FINISHED);

    // 52 shared + 6 home = 58
    t.state = TOKEN_ACTIVE;
    t.progress = 52;
    MoveToken(&t, 1);
    assert(t.progress == 53 && t.state == TOKEN_HOME);
    MoveToken(&t, 6);
    assert(t.progress == 58 && t.state == TOKEN_FINISHED);

    // Defeated token returns to base
    SendTokenToBase(&t);
    assert(t.state == TOKEN_BASE && t.progress == 0);

    // board_init fills all 52 shared squares with ids 1..52 on the cross
    Game g = {0};
    g.mode = MODE_CLASSIC;
    board_init(&g);
    assert(g.board[0].id == 1 && g.board[51].id == 52);
    assert(g.board[0].type == SQ_SAFE && g.board[5].type == SQ_NORMAL);
    for (int i = 0; i < BOARD_SIZE; i++) {
        assert(g.board[i].id == i + 1);
        assert(g.board[i].screenPos.x >= LUDO_X && g.board[i].screenPos.y >= LUDO_Y);
    }
    // Cross arm start positions (grid coords from path table)
    assert(g.board[0].screenPos.x  == LUDO_X + 0.5f * LUDO_CELL);   // square 1: (6,0)
    assert(g.board[13].screenPos.y == LUDO_Y + 0.5f * LUDO_CELL);   // square 14: (0,8)
    assert(g.board[26].screenPos.x == LUDO_X + 14.5f * LUDO_CELL);  // square 27: (8,14)
    assert(g.board[39].screenPos.y == LUDO_Y + 14.5f * LUDO_CELL);  // square 40: (14,6)

    printf("test_classic: all assertions passed\n");
    return 0;
}
#endif
