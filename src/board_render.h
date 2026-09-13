#ifndef BOARD_RENDER_H  //the beginning of a header guard, which prevents the same header from being included multiple times
#define BOARD_RENDER_H

#include "game.h"

void board_draw(Game *g);    
void board_draw_hud(Game *g);//HUD means Heads-Up Display,the HUD is the information displayed around the game screen

#endif
