#ifndef BOARD_H
#define BOARD_H

#include "game.h"

void board_init(Game *g);
Vector2 board_get_pos(BoardSquare *sq);
int board_find_square_type(Game *g, int position);
int board_get_ladder_dest(int position);
int board_get_snake_dest(int position);

// Classic Mode geometry (52-cell Ludo cross track)
int GetStartSquare(int player);                 // 1, 14, 27, 40
int GetSharedBoardSquare(int player, int progress); // logical square 1..52 for journey progress 1..52
int GetNextBoardSquare(int currentSquare);      // wraps 52 -> 1
bool IsSafeSquare(int square);

// Classic Mode token rules
bool CanDeployToken(const Token *t, int dice);  // base + roll == 6
bool CanMoveToken(const Token *t, int dice);    // no overshoot past TOTAL_TRAVEL_STEPS
bool IsOnSharedTrack(const Token *t);
bool IsInHomeLane(const Token *t);
void MoveToken(Token *t, int dice);
void SendTokenToBase(Token *t);

#endif
