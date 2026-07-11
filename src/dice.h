#ifndef DICE_H
#define DICE_H

#include "game.h"

void dice_roll(Dice *d);
void dice_update(Dice *d);
void dice_draw(Dice *d, int x, int y);

#endif // DICE_H
