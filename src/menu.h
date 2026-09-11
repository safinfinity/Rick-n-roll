#ifndef MENU_H
#define MENU_H

#include "game.h"

#define MENU_NO_ACTION   -1    // nothing clicked this frame
#define MENU_MODE_PICKED 100   // Classic/Ladder clicked, we then move to player-count screen

void menu_init(void);// menu initialize, prepares the menu for use
int menu_update(Game *g);// what the player is picking from the menu
void menu_draw(Game *g);//what should we display based on the palyers choice, w/o it we would not see the menu


#endif // MENU_H
