#ifndef BOARD_H
#define BOARD_H

#include "game.h"

void board_init(Game *g);
Vector2 board_get_pos(BoardSquare *sq);

#endif
