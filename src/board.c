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

// ── Classic board layout (48-cell ring on a 13x13 grid) ──
#define RING_N 13
#define RING_CELL 48.0f
#define RING_X 40.0f
#define RING_Y 120.0f

// ── Classic Mode geometry ──

int GetStartSquare(int player) {
    static const int starts[MAX_PLAYERS] = {1, 13, 25, 37};
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
        case 1: case 13: case 25: case 37: // starting squares
        case 5: case 17: case 29: case 41: // mid-sector star squares
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

// Screen position of logical square (1..48) on the 13x13 ring, numbered clockwise
// from the bottom-left corner. Starts: P0=1(bottom-left), P1=13(bottom-right),
// P2=25(top-right), P3=37(top-left).
static Vector2 ring_pos(int square) {
    int r, c;
    if (square >= 1 && square <= 13)      { r = RING_N - 1; c = square - 1; }
    else if (square >= 14 && square <= 24){ r = RING_N - 1 - (square - 13); c = RING_N - 1; }
    else if (square >= 25 && square <= 37){ r = 0; c = RING_N - 1 - (square - 25); }
    else                                  { r = square - 37; c = 0; } // 38..48, left side
    return (Vector2){ RING_X + (c + 0.5f) * RING_CELL, RING_Y + (r + 0.5f) * RING_CELL };
}

static Vector2 ring_cell(int r, int c) {
    return (Vector2){ RING_X + (c + 0.5f) * RING_CELL, RING_Y + (r + 0.5f) * RING_CELL };
}

// Home lanes run 6 cells from the shared-track entry straight in toward the center.
// Base yards are 2x2 clusters in the four inner corners.
static void classic_home_and_base(Game *g) {
    // entry grid coords per player (computed from GetSharedBoardSquare(p,30))
    const int sr[MAX_PLAYERS] = {0, 4, 12, 7};
    const int sc[MAX_PLAYERS] = {6, 0, 5, 12};
    const int dr[MAX_PLAYERS] = {1, 0, -1, 0}; // inward direction (toward center r=6,c=6)
    const int dc[MAX_PLAYERS] = {0, 1, 0, -1};
    // base inner-corner rows/cols
    const int br[MAX_PLAYERS][2] = {{10, 11}, {10, 11}, {1, 2}, {1, 2}};
    const int bc[MAX_PLAYERS][2] = {{1, 2}, {10, 11}, {10, 11}, {1, 2}};

    for (int p = 0; p < MAX_PLAYERS; p++) {
        int r = sr[p], c = sc[p];
        for (int i = 0; i < HOME_STEPS; i++) {
            r += dr[p];
            c += dc[p];
            g->homeLanePos[p][i] = ring_cell(r, c);
        }
        int t = 0;
        for (int i = 0; i < 2 && t < TOKENS_PER_PLAYER; i++) {
            for (int j = 0; j < 2 && t < TOKENS_PER_PLAYER; j++) {
                g->basePos[p][t++] = ring_cell(br[p][i], bc[p][j]);
            }
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
            g->board[i].screenPos = ring_pos(i + 1);
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
    // Start squares
    assert(GetStartSquare(0) == 1);
    assert(GetStartSquare(1) == 13);
    assert(GetStartSquare(2) == 25);
    assert(GetStartSquare(3) == 37);

    // Wraparound
    assert(GetNextBoardSquare(48) == 1);
    assert(GetNextBoardSquare(1) == 2);

    // Player-specific shared squares: 30 steps = 30th shared square, no home yet
    assert(GetSharedBoardSquare(0, 1) == 1);
    assert(GetSharedBoardSquare(0, 30) == 30);
    assert(GetSharedBoardSquare(1, 1) == 13);
    assert(GetSharedBoardSquare(1, 30) == 42);
    assert(GetSharedBoardSquare(2, 1) == 25);
    assert(GetSharedBoardSquare(2, 30) == 6);
    assert(GetSharedBoardSquare(3, 1) == 37);
    assert(GetSharedBoardSquare(3, 30) == 18);

    // Safe squares: starts + mid-sector stars
    assert(IsSafeSquare(1) && IsSafeSquare(13) && IsSafeSquare(25) && IsSafeSquare(37));
    assert(IsSafeSquare(5) && IsSafeSquare(17) && IsSafeSquare(29) && IsSafeSquare(41));
    assert(!IsSafeSquare(2) && !IsSafeSquare(48) && !IsSafeSquare(30));

    // Deployment: only exact 6
    Token t = {0};
    t.state = TOKEN_BASE;
    assert(!CanDeployToken(&t, 5));
    assert(CanDeployToken(&t, 6));
    assert(!CanMoveToken(&t, 6)); // base tokens cannot move

    // Overshoot rejection
    t.state = TOKEN_ACTIVE;
    t.progress = 34;
    assert(!CanMoveToken(&t, 3)); // 34+3 > 36
    assert(CanMoveToken(&t, 2));  // 34+2 == 36
    MoveToken(&t, 2);
    assert(t.progress == 36 && t.state == TOKEN_FINISHED);

    // 30 shared + 6 home = 36
    t.state = TOKEN_ACTIVE;
    t.progress = 30;
    MoveToken(&t, 1);
    assert(t.progress == 31 && t.state == TOKEN_HOME);
    MoveToken(&t, 6);
    assert(t.progress == 36 && t.state == TOKEN_FINISHED);

    // Defeated token returns to base
    SendTokenToBase(&t);
    assert(t.state == TOKEN_BASE && t.progress == 0);

    // board_init fills all 48 shared squares in a ring with ids 1..48
    Game g = {0};
    g.mode = MODE_CLASSIC;
    board_init(&g);
    assert(g.board[0].id == 1 && g.board[47].id == 48);
    assert(g.board[0].type == SQ_SAFE && g.board[6].type == SQ_NORMAL);
    for (int i = 0; i < BOARD_SIZE; i++) {
        assert(g.board[i].id == i + 1);
        assert(g.board[i].screenPos.x >= RING_X && g.board[i].screenPos.y >= RING_Y);
    }
    // ring corners land on the four start squares
    assert(g.board[0].screenPos.y == RING_Y + 12.5f * RING_CELL);          // square 1 bottom-left
    assert(g.board[12].screenPos.y == RING_Y + 12.5f * RING_CELL);         // square 13 bottom-right
    assert(g.board[24].screenPos.y == RING_Y + 0.5f * RING_CELL);          // square 25 top-right
    assert(g.board[36].screenPos.y == RING_Y + 0.5f * RING_CELL);          // square 37 top-left

    printf("test_classic: all assertions passed\n");
    return 0;
}
#endif
