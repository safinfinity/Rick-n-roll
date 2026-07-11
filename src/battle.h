#ifndef BATTLE_H
#define BATTLE_H

#include "game.h"

void battle_draw(Game *g);
void battle_update(Game *g);
void init_battle(Game *g, int attacker, int defender);

#endif // BATTLE_H
