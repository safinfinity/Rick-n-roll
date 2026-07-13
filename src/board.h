#ifndef BOARD_H
#define BOARD_H

#include "game.h"

void board_init(Game *g);
Vector2 board_get_pos(BoardSquare *sq);
int board_find_square_type(Game *g, int position);
int board_get_ladder_dest(int position);
int board_get_snake_dest(int position);

#endif
