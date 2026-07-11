#include "game.h"
#include "board.h"
#include <string.h>
#include <math.h>

#define BOARD_COLS 6
#define BOARD_ROWS 5
#define CELL_SIZE 100
#define BOARD_X 60
#define BOARD_Y 140

void board_init(Game *g) {
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
