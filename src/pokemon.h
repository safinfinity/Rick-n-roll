#ifndef POKEMON_H
#define POKEMON_H

#include "game.h"

Pokemon poke_create(const char *name, PokeType type);
void poke_assign_random(Player *players, int count);

#endif // POKEMON_H
