#ifndef MENU_H
#define MENU_H

#include "game.h"

#define MENU_NO_ACTION   -1    // nothing clicked this frame
#define MENU_MODE_PICKED 100   // Classic/Ladder clicked, we then move to player-count screen

void menu_init(void);
int menu_update(Game *g);
void menu_draw(Game *g);

#endif // MENU_H
